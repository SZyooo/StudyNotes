# HTTPS/Websocket中的加密

## 一、HTTP和HTTPS

HTTP是明文协议，HTTPS则是在HTTP的基础上引入了SSL/TLS协议。

其中，SSL（Secure Sockets Layer）是TLS（Transport Layer Security）的前身，两者都是加密协议，SSL已经被淘汰。TLS有两个版本：1.2和1.3。

### TLS 握手过程

这里以TLS1.2为例，讲解一下TLS是怎么进行加密的。

0. TCP三次握手
1. ["client hello"]客户端发送“client hello”文字，同时加上支持的TLS版本、加密套件以及一个随机数A给服务端
    （加密套件包括客户端支持的加密算法等）
2. ["server hello"]服务端收到客户端消息之后，确定TLS版本以及所选择的加密套件中的加密算法；同时也生成一个随机数B给客户端
3. ["certificate"]服务器继续发送自己的服务器证书给客户端，客户端通过该证书来鉴定服务器是否是自己访问的目标服务器
4. ["server key exchange"]紧接着服务器继续发送自己的公钥给客户端
5. ["server hello down"]服务器发送“server hello done”，告知发送结束
6. ["client key exchange"] 客户端生成第三个随机数C，并且用服务器发送的公钥进行加密发送给客户端
7. ["Change Ciper Spec"] 客户端发送给服务器，确认加密套件。接下来发送的都是加密的消息了
8. ["Encryped Handshadke Message"] 客户端发送给服务器
9. ["Encryped Handshake Message"] 服务器发送给客户端，TLS握手完成

这个过程中，三个随机数有什么作用？答案是生成会话密钥。A和B随机数是明文的，所以双方都知道；C随机数是客户端生成的，然后用服务器的公钥加密之后发送给服务器，服务器用自己的私钥解密，这样的话，两方也都知道了C随机数。

接着两方用这3个随机数生成会话密钥，后面的通信就都是基于这个会话密钥进行对称加密的了。

## 二、CA

客户端通过服务器发送的证书来验证连接的对象是不是自己想要连接的目标服务器。这个过程是怎么完成的？

这个过程需要有一个权威机构来对服务器发送的证书进行签名，以自己的权威身份认证这份服务器发过来的证书是自己确认过的。

具体过程是：

1. 权威机构（Certification Authority）具有一个私钥和一个公钥。服务器申请CA签名的时候，CA会将服务器的证书进行一个类似哈希的操作，得到一串哈希字符串，然后用自己的私钥加密，添加到服务器的证书上
2. 客户端接收到了服务器的证书之后，将签名之外的部分也进行哈希操作，得到字符串A；接着用CA的公钥对证书的签名部分进行解密，得到字符串B。如果A==B，则说明B确实是CA的签名，则认证成功；否则认证失败

## 三、本地部署CA

> 更详细的本地本地配置CA和生成服务器key/crt请查看[本地为Redis生成key](./本地生成CA并且给Redis服务签发证书.md)

我们在自己开发的服务器和客户端之间进行Websockets或者HTTPS通信的时候，可以引入TLS加密。我们不必向权威的CA申请签名，我们可以自己部署一个CA，然后将CA添加到我们的客户端认可的cA列表中。

这里以OpenSSL库为例演示。

### 3.1 生成CA

首先是生成CA。CA主要生成两个文件：一个是私钥，一个是公钥

```bash
openssl genrsa -out ca.key 4096
```
这个命令是生成4096位的RSA私钥，名为`ca.key`

```bash
openssl req -x509 -new -nodes -key ca.key -sha256 -days 3650 -out ca.crt
```
在私钥ca.key的基础上，生成一个使用sha256算法，时间位3650天的自签名X.509证书，名字叫ca.crt(Certificate)。
这个ca.crt里面包含的就是服务器的公钥、身份信息等

### 3.2 为服务器生成证书

接着我们需要用我们自己的CA给服务器生成证书

```bash
# 生成私钥文件 server.key
openssl genrsa -out server.key 2048
# 基于私钥生成证书签名请求文件 server.csr
openssl req -new -key server.key -out server.csr
```

其中csr(Certificate Signing Request)是请求签名的文件。生成之后我们让CA进行签名：

```bash
openssl x509 -req -in server.csr -CA ca.crt -CAkey ca.key -CAcreateserial -out server.crt -days 365 -sha256
```
这条命令的含义是根据server.csr请求，用ca.crt和ca.key生成签名，输出是server.crt

### 3.3 在客户端配置好CA

我们需要将我们的CA添加到客户端的认证中，这样客户端就能用我们的CA来对服务器的证书进行验证。这里以Libwebsockets为例：

```c
// 1. 定义协议信息
static struct lws_protocols protocols[] = {
    {
        "my-protocol", // 协议名称
        callback,      // Websocket 回调函数
        sizeof(struct per_session_data__minimal), // 会话数据大小
        4096,          // 接收缓冲区大小
    },
    { NULL, NULL, 0, 0 } // 终止标志
};

// 2. 配置连接信息 (关键步骤)
struct lws_client_connect_info i;
memset(&i, 0, sizeof(i));

i.context = context;
i.address = "your.server.domain.com";
i.port = 443;
i.path = "/";
i.host = i.address;
i.origin = i.address;
i.ssl_connection = LCCSCF_USE_SSL; // 启用 SSL

// 设置 CA 证书路径和客户端证书（如有）
i.client_ssl_ca_filepath = "/path/to/your/ca.crt"; // CA 证书关键路径，用于验证服务器
i.client_ssl_cert_filepath = "/path/to/your/client.crt";
i.client_ssl_private_key_filepath = "/path/to/your/client.key";

// 在实际项目中，还可以根据需要设置选项
// i.ssl_connection |= LCCSCF_ALLOW_SELFSIGNED; // 仅测试用，允许自签名
// i.ssl_connection |= LCCSCF_SKIP_SERVER_CERT_HOSTNAME_CHECK; // 仅测试，跳过主机名检查[reference:9]

// 3. 发起连接
struct lws *wsi = lws_client_connect_via_info(&i);
```

## 四、什么是自签名

实际上存在一些中间CA，它们并不是绝对的CA，因此它们也需要更高一级的CA进行签名。换句话说，CA本身也要证明自己是可信的。除了根CA是绝对可信的外，其他CA都需要直接或者间接的被根CA签名。

但是对于根CA而言，它不需要更高的CA证明，因此它直接对自己的证书用自己的私钥签名。这就是自签名。

如果我们取消注释 `// i.ssl_connection |= LCCSCF_ALLOW_SELFSIGNED;`之后，客户端会接收自签名的服务器证书。这意味着这个证书的CA是它自己。
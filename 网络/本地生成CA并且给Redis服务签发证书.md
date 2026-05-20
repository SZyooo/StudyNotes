# 样例：本地生成CA并且给Redis签发证书

## 一、本地生成CA

### 1.1 先生成必要的目录

```bash
mkdir myCA && cd myCA
mkdir newcerts
mkdir private
echo 01> serial     #序列号起始
touch index.txt     #记录以签发证书的文件
```

### 1,2 OpenSSL配置文件

用于指定CA签发时候的默认参数

```ini
[ ca ]
default_ca = my_ca

[ my_ca ]
dir		= .	
# 新证书存放的位置
new_certs_dir = ./newcerts		
# 每次签名的记录
database = ./index.txt		
# 每张证书的序号文件
serial = ./serial		  
# CA证书  
certificate = ./private/ca.crt	
# CA私钥
private_key = ./private/ca.key	
default_md = sha256
policy = policy_anything
# 不复制CSR的扩展。设置为copy的话会将CSR中的扩展复制到证书中，可能会导致一些不必要的扩展被添加到证书中。
copy_extensions = none              
[ policy_anything ]
countryName = optional
stateOrProvinceName = optional
localityName = optional
organizationName = optional
organizationalUnitName = optional
commonName = supplied
emailAddress = optional

[ req ]
default_bits = 2048
distinguished_name = req_distinguished_name
string_mask = utf8only
default_md = sha256
prompt = no

[ req_distinguished_name ]
commonName = TexasHoldemYSZ CA

# 这个字段用来配置CA根证书的X.509 v3扩展属性配置
[ v3_ca ]
# 告诉验证方这是一个CA，可以用来签发其它证书
basicConstraints = critical, CA:TRUE
# 告知证书的唯一合法用途：签发证书（keyCertSign），签发证书吊销列表（cRLSign，certificate revocation list）	         
keyUsage = critical, keyCertSign, cRLSign     

# 指定自身的唯一编码。这里会对当前证书的公钥进行hash计算，生成一个唯一的ID写入证书
subjectKeyIdentifier = hash
# 指定CA的签发者。对于根证书来说签发者是自己
# keyid : always - 直接复制上级CA证书的subjectKeyIdentifier，构成父子关系链；并且强制要求这个值必须写入 issuer：附带CA的名称等信息
authorityKeyIdentifier = keyid:always, issuer

[ server_cert ]
basicConstraints = CA:FALSE
keyUsage = digitalSignature, keyEncipherment
extendedKeyUsage = serverAuth
subjectKeyIdentifier = hash
authorityKeyIdentifier = keyid:always,issuer
subjectAltName = @alt_names

[ alt_names ]
DNS.1 = localhost
IP.1 = 127.0.0.1

```
### 1.3 生成CA的私钥和根证书

```bash
# 生成私钥
openssl genrsa -out private/ca.key 2048

# 用这个证书生成自签名的根证书，有效期10年
openssl req -config openssl.cnf -new -x509 -days 3650 \
    -key private/ca.key -out private/ca.crt \
    -extensions v3_ca
```

## 二、为服务器生成私钥和证书

```bash
# 生成私钥
openssl genrsa -out redis.key 2048

# 创建证书签名请求（CSR）
openssl req -new -key redis.key -out redis.csr -subj "/CN=your-redis-host.local"
```

> 这里的`/CN`并不需要和前面的openssl.cnf中的CommonName相等，但是必须给出，因为`policy_anything`指定`commonName = supplied`。

给生成redis.crt添加扩展配置文件：

```ini
authorityKeyIdentifier=keyid,issuer
basicConstraints=CA:FALSE
keyUsage = digitalSignature, keyEncipherment
extendedKeyUsage = serverAuth
subjectAltName = @alt_names

[alt_names]
DNS.1 = localhost
IP.1 = 127.0.0.1
```

## 三、为Redis公钥签名

```bash
openssl ca -config openssl.cnf -in redis.csr -out redis.crt \
    -extfile redis.ext -days 825 -notext -batch
```

## 四、配置Redis

编辑redis.conf:

```ini
tls-port 6379
port 0
tls-cert-file /path/to/redis.crt
tls-key-file /path/to/redis.key
tls-ca-cert-file /path/to/myCA/private/ca.crt   # 用于验证客户端（如果开启mTLS）
tls-auth-clients no          # 若不要求客户端证书，设为 no
```

## 五、在C++客户端中添加crt

```c++
// 伪代码示例
redis::ConnectionOptions opts;
opts.host = "127.0.0.1";
opts.port = 6379;
opts.tls.enabled = true;
opts.tls.cacert = "/path/to/myCA/private/ca.crt";   // 信任的根证书
opts.tls.cert = "";   // 如果不要求客户端证书，留空
opts.tls.key = "";
opts.tls.sni = "your-redis-host.local";    // 对应证书的SAN
```

> SAN: Subject Alternative Name，就是在`[alt_name]`里面的条目
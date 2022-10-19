## mit6.828环境搭建

系统：ubuntu 18.04.4

参考网址：https://pdos.csail.mit.edu/6.828/2018/tools.html

### 工具准备

下载`git`

```shell
sudo apt install git
```

若报错可尝试`aptitude`工具

### 环境配置

- ```shell
  objdump -i
  gcc -m32 -print-libgcc-file-name
  ```

​		这两步若是没有出错，则直接进入克隆环节。

- 用`git`工具clone `QEMU`

  ```shell
  git clone https://github.com/mit-pdos/6.828-qemu.git qemu
  ```

  若无法连接访问Github

  试试下面的两条命令

  ```shell
  git config --global  --unset https.https://github.com.proxy 
  git config --global  --unset http.https://github.com.proxy 
  ```

- 安装一些packages

  ```shell
  sudo apt install libsdl1.2-dev
  sudo apt install libtool-bin
  sudo apt install libglib2.0-dev
  sudo apt install libz-dev
  sudo apt install libpixman-1-dev
  ```

  若出错，安装`aptitude`工具。

- 进行config

  ```shell
  ./configure --disable-kvm --disable-werror --target-list="i386-softmmu x86_64-softmmu"
  ```

- 最后

  ```shell
  make  					# 耐心等待一下
  make install
  # 若出错
  sudo make install
  ```

#### 安装JOS

- 首先安装一下32位的库

  ```shell
  sudo apt-get install gcc-multilib
  ```

- git命令

  ```shell
  git clone https://pdos.csail.mit.edu/6.828/2018/jos.git lab
  ```

  - ​	进入`lab`文件夹

    ```shell
    make qemu
    # 若出错
    make qemu-nox
    ```

    若成功，则会出现一个新的命令行。

    按`ctrl+a x`退出。
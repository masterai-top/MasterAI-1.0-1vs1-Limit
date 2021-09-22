编译方法：-- 参考网页：http://blog.sina.com.cn/s/blog_70508e7f01015lkz.html
1、解压
    tar -xvf jsoncpp-src-0.5.0.tar.gz
    unzip scons-3.1.1.zip

2、设定环境变量
    export MYSCONS=SCONS的解压的路径  (export MYSCONS=/home/zhonghailong/src/DT_Server/Common/jsoncpp/scons-3.1.1)
    export SCONS_LIB_DIR=$MYSCONS/engine

3、开始编译jsoncpp
    cd jsoncpp-src-0.5.0
    python $MYSCONS/script/scons platform=linux-gcc

    之后编译，生成静态和动态库文件, 查看jsoncpp-src-0.5.0/libs/linux-gcc-***/目录, 有如下文件：
        libjson_linux-gcc-***_libmt.a
        libjson_linux-gcc-***_libmt.so
    
    重要：最后删除.so文件: 直接使用静态库。(若需要使用.so,则需要配置系统配置文件)
    
    
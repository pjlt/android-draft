# android-draft

 这是一个非正式项目。

 主要是作者之前几乎没有安卓开发经验，只做过一个C++库，以纯C接口提供给专门的安卓开发使用。

 在正式开发lanthing-android之前，便有了这个项目，承担学习和预研的职责。边看安卓教程边学kotlin边开发，花了7、8天将lanthing-pc移植到了安卓上。当前版本做到的、和没做到的，大致如下描述：

- [x] 用设备码和验证码，作为主控成功连接lanthing-pc
- [x] 有画面
- [x] 有声音
- [x] 硬解hevc
- [x] 硬解avc
- [ ] 能操作
- [ ] 可以使用Github Actions编译
- [ ] 作为被控

如果这是个纯Kotlin/Java应用，编译工作将简单许多，然而从Github页面可以看出，这是个C++占到3/4的安卓应用，安卓层面主要是Kotlin + Jetpack Compose，剩下的还有少量从lanthing-svr移植过来的Java代码。

lanthing-pc的C++代码，从Windows端移植到Android上有点麻烦、但也不算是太难的事。移植这些C++代码所依赖的第三方库才是一件令人头疼的事。比如C++版的Protobuf，我们除了需要用到libprotobuf.so外，还需要protoc去生成.pb.cc和.pb.h，也就是说需要保持libprotobuf.so是arm64架构的同时，让protoc是编译机的架构（比如Linux和Windows下的x86），这显然不是一条CMake命令就能完成的事。此外lanthing所使用的传输库librtc.so的编译也是一件相当麻烦的事，这里就不多赘述。

这些依赖库，作者还没能利用Github Actions在线编译，只在本地手动编译，拷贝到对应的开发目录，没有上传到Github。所以你clone下这个项目只也能“看”，无法“编译运行”。

这个承担学习和预研职责的demo，将告一段落，接下来进入正式的安卓端开发。新项目暂时闭源开发。

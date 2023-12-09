protoc -I=proto\ --cpp_out=..\..\CppCommon\include\ckutil\packet\ proto\*.proto
protoc -I=proto\ --csharp_out=..\CsharpPacket\Packet\ proto\*.proto
@pause
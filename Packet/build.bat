set pathCpp="C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin"


ProtoGen\protoc -I=ProtoGen\proto\ --cpp_out=..\CppCommon\include\ckutil\packet\ ProtoGen\proto\*.proto
ProtoGen\protoc -I=ProtoGen\proto\ --csharp_out=CsharpPacket\Packet\ ProtoGen\proto\*.proto

%pathCpp%\msbuild CppPacket\Packet.sln

%pathCpp%\msbuild CsharpPacket\CsharpPacket.sln 

@pause
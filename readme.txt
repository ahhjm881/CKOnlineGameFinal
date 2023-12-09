패킷 추가/변경/삭제 하는 법
1. Packet\ProtoGen\proto\*.proto 파일들 추가/변경/삭제
2.  Packet\ProtoGen\gen.bat를 이용하여 .proto 파일 컴파일
3. 현재 실행된 vs 프로젝트가 있다면, c++ 프로젝트의 경우 인텔리센스가 작동안 할 수 있음. 그럴 경우 껏다키기

+ 추가로 .proto 파일 자체가 추가되었다면, Projects/MainServer, Packet/csharpPacket 프로젝트에 각각 컴파일된 소스코드들 포함시켜줘야함.
.cs 파일로 컴파일 된 것은 CsharpPacket\Packet 폴더에 복사됨
.h, .cc 파일로 컴파일 된 것은Projects\MainServer\Packet 폴더에 복사됨

빌드 이벤트를 사용하여 관련된 .h, lib, dll와 같은 출력 파일들을
서버 프로젝트, 클라이언트 프로젝트로 복사하도록 해놓았음.
그래서 그냥 실행하면 서버/클라이언트에서 수정된 패킷 관련 코드들이 일괄적으로 자동 적용됨.


build.bat 사용법과 쓰는 이유
cshar, cpp packet 프로젝트와 proto 컴파일과 빌드해주는 스크립트다.
딱히 사용 안 해도 됨.

visual studio installer에서 .net 부분의 "이식 가능한 타겟팅 팩.." 어쩌구를 설치한다.

build.bat 파일에서 {set pathCpp="C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin"}
부분을 본인의 msbuild 폴더 위치에 맞게 수정한다.

!!!!!위 내용을 따라할 수 없다면 .proto 파일을 수정하여 패킷을 추가/변경/삭제 할 때 마다,
해당 폴더의 언어별(c++, c#)packet 라이브러리의 솔루션 파일을 열고 일일이 빌드해준다.!!!!
Chat Server
===========
**IOCP**를 사용한 비동기 방식의 채팅 서버입니다.

## 변수 명명법  
* 멤버 변수는 **파스켈 케이스**, 지역 변수(매개 변수 포함)은 **카멜 케이스**를 사용했습니다.  

* 상수는 구글에서 만든 Naming Rule을 참고해 k 접두어로 시작하는 **파스칼 케이스**을 적용했습니다.  

* 매크로와 enum class의 멤버는 언더바를 이용해 의미를 구분했습니다. 전부 대문자입니다. 

* 약어 사용을 최소화했습니다. 변수 이름에서 역할과 기능을 잘 보여주는 것에 주안점을 뒀습니다. 변수 이름이 너무 길어지거나 클래스 이름이 변수 이름에 가장 적합하다고 생각될 경우에 약어를 사용했습니다.

## 로그  
* conmanip.h라는 라이브러리를 사용해 경고, 디버깅 등 유형에 따라 색상을 바꿔 콘솔에 출력합니다. 기존 라이브러리는 출력에 cout을 사용했지만 로깅에 들어가는 자원 소모를 줄이기 위해 printf 함수로 변경했습니다.  

* 헤더파일에 함수 구현부까지 있기 때문에 ILog라는 추상함수를 만들어 사용해야 오브젝트 파일이 링크되는 과정에서 오류가 발생하지 않습니다.

## 클라이언트  
* C#으로 작성했습니다. **다른 언어끼리 통신하는 것**을 연습하기 위해서며 C#의 생산성을 경험해보는 것이 필요하다 생각했습니다. 
WinAPI를 다뤄본 적 있기 때문에 폼 작성에 큰 어려움은 없었습니다. 네트워킹과 스레드 사용에서 C#의 강력함을 느낄 수 있었습니다.  

* 데이터를 byte 단위로 변환하거나 수신한 byte[] 데이터에서 원하는 위치에 원하는 만큼 데이터를 추출하는 함수가 지원되는 점이 편했습니다.

## 서버
* STL의 엄격함을 경험할 수 있었습니다. UserIdDic은 unordered_map으로 선언한 변수며 키가 const char* 이었고 밸류가 User* 입니다.
FindUser 함수에서 id를 인자로 받아 UserIdDic.find(id)을 통해 반복자를 얻어야 하는데, id를 string으로 넣으니 작동하지 않았습니다.  

* 많이는 아니지만 람다 표현식을 쓸 수 있는 곳 일부에는 람다 표현식을 썼습니다.  

* lock은 **유저모드 동기화 기법**을 사용했습니다. 카운트와 관련된 변수는 Inter 계열의 함수가 아닌 C++ 문법인 **atomic**을 활용했습니다. 어셈블리에서 해당 구문에 직접 lock이 걸리기 때문에 속도가 빠릅니다. 아직 제 수준에서는 뮤텍스의 기능을 쓸 일이 없어 크리티컬 섹션으로 lock 클래스를 새로 구현해 사용했습니다.  

* 패킷의 아이디에 따른 함수 호출은 <functional>을 사용했습니다. 키는 패킷 아이디, 밸류는 함수 포인터인 unordered_map 변수를 선언했습니다. 관리를 편하기 하기 위해 패킷에 따른 처리를 소스 파일로 분리했습니다.
  
* Main 클래스에서 클래스들의 인스턴스를 만드는데, 안전하게 포인터 변수 사용을 위해 unique_ptr을 사용했습니다.  

* Message라는 구조체가 있습니다. WokrerThread에서 수신한 데이터를 처리하기 위해 Logic Completion Port로 보내는데, 이때 타입을 지정해줘야 올바른 처리를 해줄 수 있으므로 Message 구조체에 char* 변수와 타입 변수 선언해 사용합니다. Message를 사용하기 위해서는 MessagePool에서 할당받아야 구조인데, 멀티스레드 환경이므로 **공유자원 접근 문제**가 발생합니다. 락을 걸어도 되지만, 문법에서 지원하는 기능을 쓰는 것이 성능에 이점이 있어 MessagePool은 concurrent_queue< Message* >로 선언된 멤버 변수를 가지고 있습니다.

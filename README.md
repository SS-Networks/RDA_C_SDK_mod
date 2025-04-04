C SDK SAMPLE
=




## 1. 개요

**C SDK**는 사물에서 **Brightics IoT 사물 관리 기능, Open API** 등 플랫폼 연계 기능을 쉽게 개발하도록 제공하는 C 프로그래밍 라이브러리 입니다.

본 가이드에서 제공하는 샘플 코드를 활용하여 C SDK를 통해 Brightics IoT와 장비의 연결상 상태 확인 및 샘플 데이터를 전송하여 정상적으로 데이터가 수집되는지 확인하는것이 목표입니다.
## 2. 개발환경 구성하기

개발환경 구성은 아래 매뉴얼 링크를 클릭하여 진행 해주시기 바랍니다.
> - [samsung C SDK 매뉴얼 링크](https://www.samsungsdsbiz.com/help/brighticsiot/core/V3/3-0/KR/sdk/using_c_sdk#configuring_the_development_environment_for_c_sdk)  
  ※ 최소사양 (OS - Linux 계열) 권장 하드웨어 (라즈베리 파이 3 이후 모델)

## 3. 샘플 활용하기

삼성에서 제공해주는 SDK를 활용하여 PoC 환경에서 동작할 수 있도록 샘플 코드를 수정 하였습니다.  
아래 링크를 클릭 하여 가이드 문서를 내려받아 '샘플 활용' 부분을 확인 해주세요.
> - [가이드 문서 다운로드](./guide/SDK_GUIDE_C.hwp)

본 문서에는 2개의 샘플 활용 예제가 있습니다.  
> - ITA 인증방식으로 장비 연결 확인하기 (Connection.c)  
> - ITA 인증방식으로 장비 연결 후 샘플 데이터 전송하기 (Attribute.c) 

이 외에 다른 파일도 수정하여 동일하게 활용할 수 있습니다.
###
### 전송한 샘플데이터는 PoC 사물 속성 모니터링에서 확인할 수 있습니다.
![image](./guide/image/monitoring.png)

## 4. 참고사항
- 인증 방식 중 ITA만 샘플 코드를 작성하였습니다.
- 인증에는 사이트 아이디, 장비별 인증코드, 사물 명이 필수 파라미터이며, 인증에 아이디/패스워드는 사용하지 않으므로 제공하지 않습니다.
- C SDK는 기본적으로 인증서를 찾는 로직이 있으므로, SSL 방식이 아니더라도 샘플 코드에 포함된 인증서의 경로를 설정파일에 반드시 입력 해야 합니다.

## 자주 묻는 질문
- [SDK 실행 전 확인해야 할 사항들](https://github.com/RDA-DATA/RDA_C_SDK/issues/5)
- [패키지 설치가 안되는 현상](https://github.com/RDA-DATA/RDA_C_SDK/issues/6)
- ["Can`t locate FindBin.pm in @inc ... " 에러가 발생하는 현상](https://github.com/RDA-DATA/RDA_C_SDK/issues/7)
- [vi 명령어를 사용했으나 수정하기 어려운 상태이거나 "command not found" 라는 오류 메시지가 출력되는 현상](https://github.com/RDA-DATA/RDA_C_SDK/issues/8)
- [curl 설치중 ./configure 명령어 실행시 에러가 발생하는 현상](https://github.com/RDA-DATA/RDA_C_SDK/issues/9)

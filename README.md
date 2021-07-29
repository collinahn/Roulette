# Roulette

### Description
* Creates roulette module using cocos2d-x physics engine.

* Stops at predetermined area. (current: random number)

* The number of roulette's element can also be adjusted.

* Trial Video -> [Link](https://youtu.be/h7nct3X7VBU)

### Environment
* M1 Mac mini, 8GB

* MacOS Big Sur 11.4 

* x86_64 iOS simulaor built by Xcode


### Prerequisite
* Xcode 12.6

* cocos2d-x 3.17

* Xcode libcocos iOS setting :
  * $(archs): arm64, x86_64
  * $(excluded_archs): arm64

* Podfile setting(if x86_64 based library is used):
```
post_install do |installer|
  installer.pods_project.build_configurations.each do |config|
    config.build_settings["ARCHS[sdk=iphoneossimulator*]"] = "x86_64"
    config.build_settings["EXCLUDED_ARCHS[sdk=iphoneossimulator*]"] = "arm64"
    config.build_settings["ONLY_ACTIVE_ARCH"] = "YES"
```
  

### Files
* In Roulette.cpp,

```c++
    //1-2. 룰렛을 n등분으로 구분하는 기둥을 만들고 배치한다
    const int n_TotalElementNumber = ELEMENT_NUM;
    const float f_AngleOnce = 360.0f / n_TotalElementNumber;
    const float f_RadiusPillarRatio = 8.0f / 9.0f;                      //룰렛 중심으로부터의 구분 기둥 위치
    cocos2d::Sprite *sprite_RouletteSpliter[n_TotalElementNumber];
    cocos2d::PhysicsBody *phbody_RouletteSpliter[n_TotalElementNumber];

    int i = 0;
    for ( auto& item : sprite_RouletteSpliter) {
        item = Sprite::create("res/button.png");
#if _REAL_MEASURE_
        item->setScale(0.05f);
#endif

        //룰렛의 중심을 기준으로 12시부터 시계 반대방향으로 차례로 배치
        item->setPosition(Point(size_Roulette.width/2.0f * ( 1.0f + f_RadiusPillarRatio*cos( (90.0f+f_AngleOnce*i)*PI/180.0f ) ),
                                size_Roulette.height/2.0f * ( 1.0f + f_RadiusPillarRatio*sin( (90.0f+f_AngleOnce*i)*PI/180.0f ) )));
        phbody_RouletteSpliter[i] = PhysicsBody::createCircle(item->getContentSize().width/2,
                                                              PhysicsMaterial(1.0f,0.0f,1.0f));
        phbody_RouletteSpliter[i]->setDynamic(false);
        phbody_RouletteSpliter[i]->setMass(10.0f);
        item->setPhysicsBody(phbody_RouletteSpliter[i]);

        phbody_RouletteSpliter[i]->setCollisionBitmask(i+1);   //0번은 마스크 1
        phbody_RouletteSpliter[i]->setContactTestBitmask(true);
        sprite_RouletteBase->addChild(item);
        i++;
    }
```
* Once the number of pillar is given, location and attribute is set automatically on each physical body. 

* Then each collides with the pointer sprite, demonstrating natural reaction. 

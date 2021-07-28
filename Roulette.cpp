//
// Roulette.cpp
//
//  1. 기능 :  chipmunk 물리엔진을 이용하여 룰렛의 회전 모션을 보인다.
//            원판의 특정 부위와 화살표가 닿으면 충돌 체크를 하여 화살표가 휘는 모션을 보이도록 한다.
//            미리 결과를 정해놓고 룰렛이 속도를 줄였을 때 그 곳에서 멈추도록 한다.
//
//  2. 이력 :  1) 2021.07.14 created by taeyoung
//            2) 2021.07.15 modifed by taeyoung: 룰렛 요소 추가. 매크로 ELEMENT_NUM 변경으로 요소 개수 수정할 수 있도록 함.
//                                               자동으로 룰렛 스프라이트 위 위치 계산하여 n등분하고 각 위치에 스프라이트와 피직스바디 추가.
//                                               화살표만 중력의 영향에 있고 화살표와 요소 구분 스프라이트가 충돌할 시 걸리는 모션 추가.
//            3) 2021.07.16 modifed by taeyoung: sprite의 크기와 physicsbody 생성 시 물리적 특성을 조정해 자연스러운 모션을 보일 수 있도록 함.
//                                               좌표상의 하드코딩을 빼고 어느 화면 크기에서도 같게 시뮬레이션되도록 상대적 비율로 크기 및 위치를 정함.
//                                               (단, 스프라이트의 크기를 기준으로 위치를 입력받는 경우 전처리기 define을 통해서 setScale가
//                                                    필요할 때와 아닐 때를 구분해 놓음)
//            4) 2021.07.19 modifed by taeyoung: 필요엾는 터치이벤트리스너 삭제.
//                                               충돌이벤트리스너 생성하여 몇 번 노드와 충돌하였는지 체크하고 메뉴를 선택함과 동시에 받아 둔 랜덤한 숫자와
//                                                    같다면 터치가 끝난 시점에 적정한 수준의 Damping 속성을 부여해서 자연스럽게 멈출 수 있도록 함
//                                               그 외 물리적 특성 조절로 화살표의 충돌이 자연스럽게 구현될 수 있도록 조정 및 코드 가독성 향상
//

#include <math.h>
#include "Roulette.h"
#include "SimpleAudioEngine.h"

USING_NS_CC;

cocos2d::Scene * Roulette::createScene() {
    auto scene = cocos2d::Scene::createWithPhysics();

#if _VIS_DEBUG_
    //물리 오브젝트의 판정범위 시각화
    scene->getPhysicsWorld()->setDebugDrawMask(PhysicsWorld::DEBUGDRAW_ALL);
#endif
    
    //중력 설정 -> 화살표만 영향 받음
    scene->getPhysicsWorld()->setGravity(Vec2(0.0f,-980.0f));
    
    auto layer = Roulette::create();
    layer->SetPhysicsWorld(scene->getPhysicsWorld());
    scene->addChild(layer);
    return scene;
}

bool Roulette::init() {
    if ( !Scene::init() ) {
        return false;
    }

    return true;
}

void Roulette::onEnter() {
    Scene::onEnter();
    
    //0. 메뉴 생성
    CreateMenu();
    //1. 룰렛 요소 생성 2. 룰렛 화살표 생성(<--크기 문제 떄문에 한번에)
    SetObjectItem();
    
    // 3. 충돌 이벤트리스너 생성
    auto listener_ContactEvent = EventListenerPhysicsContact::create();
    listener_ContactEvent->onContactBegin = CC_CALLBACK_1(Roulette::onContactBegin, this);
    
    listener_ContactEvent->onContactSeparate = CC_CALLBACK_1(Roulette::onContactSeperate, this, &mn_TargetRandNum, &mb_RouletteStopFlag);
    
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener_ContactEvent, this);
}


void Roulette::onExit() {
    _eventDispatcher->removeAllEventListeners();

    Scene::onExit();
}

void Roulette::SpinRoulette() {
    mphbody_RouletteBase->setDynamic(true);
    
    // 회전 속도 설정(제한은 ~Limit(float)으로 설정
    mphbody_RouletteBase->setAngularVelocity(-10.0f);
    // 회전 제동력 0.0f ~ 1.0f
    mphbody_RouletteBase->setAngularDamping(0.0f);
}

void Roulette::_StartSpin(cocos2d::CCObject* pSender) {
#if _DEBUG_
    CCMenuItem* pItem_Selected = reinterpret_cast<CCMenuItem*>( pSender );
    log( "%d selected, spin start, velocity -10.0f", pItem_Selected->getTag() ); //1
#endif
    
    SpinRoulette();
}

void Roulette::_SlowSpin(cocos2d::CCObject* pSender) {
#if _DEBUG_
    CCMenuItem* pItem_Selected = reinterpret_cast<CCMenuItem*>( pSender );
    log( "%d selected, set velocity -3.0f", pItem_Selected->getTag() ); //2
#endif
    
    mphbody_RouletteBase->setAngularVelocity(-3.0f);
}

void Roulette::_SetModestFriction(cocos2d::CCObject* pSender) {
#if _DEBUG_
    CCMenuItem* pItem_Selected = reinterpret_cast<CCMenuItem*>( pSender );
    log( "%d selected set damping 0.3f", pItem_Selected->getTag() ); //3
#endif
    
    mphbody_RouletteBase->setAngularDamping(0.3f);
}

void Roulette::_SetMaxFriction(cocos2d::CCObject* pSender) {
#if _DEBUG_
    CCMenuItem* pItem_Selected = reinterpret_cast<CCMenuItem*>( pSender );
    log( "%d selected, set damping 5.0f", pItem_Selected->getTag() ); //4
#endif
    
    mphbody_RouletteBase->setAngularDamping(5.0f);
}

void Roulette::_StopAbrupt(cocos2d::CCObject* pSender) {
#if _DEBUG_
    CCMenuItem* pItem_Selected = reinterpret_cast<CCMenuItem*>( pSender );
    log( "%d selected, set dynamic false", pItem_Selected->getTag() ); //5
#endif
    
    mphbody_RouletteBase->setDynamic(false);
}

void Roulette::_InitObj(cocos2d::CCObject* pSender) {
#if _DEBUG_
    CCMenuItem* pItem_Selected = reinterpret_cast<CCMenuItem*>( pSender );
    log( "%d selected, restart scene", pItem_Selected->getTag() ); //6
#endif
    
    auto scene_New = Roulette::createScene();
    Director::getInstance()->replaceScene(scene_New);
}

//타겟 숫자를 지정하고 룰렛을 일정한 속도로 돌린다.
void Roulette::SpinRouletteWithNumberSelection(cocos2d::CCObject* pSender) {
#if _DEBUG_
    CCMenuItem* pItem_Selected = reinterpret_cast<CCMenuItem*>( pSender );
    log( "%d selected, set & spin", pItem_Selected->getTag() ); //10
#endif

    srand(time(NULL));
    mn_TargetRandNum = rand()%ELEMENT_NUM + 1;
#if _DEBUG_
    log( "Number Selected : %d", mn_TargetRandNum );
#endif
    
    // 움직이도록, 회전속도(-값, 작을수록 빠름), 제동력 설정
    mphbody_RouletteBase->setDynamic(true);
    mphbody_RouletteBase->setAngularVelocity(-3.0f);
    mphbody_RouletteBase->setAngularDamping(0.0f);
}

//지정된 구역에서 정지하도록 플래그값을 세팅한다.
void Roulette::StopRouletteAtNumberSelected(cocos2d::CCObject* pSender) {
#if _DEBUG_
    CCMenuItem* pItem_Selected = reinterpret_cast<CCMenuItem*>( pSender );
    log( "%d selected, stop after 1s", pItem_Selected->getTag() ); //11
#endif
    
    mphbody_RouletteBase->setAngularVelocity(-1.5f);
    //버튼을 클릭하고 2초뒤에 플래그값 전달
    this->scheduleOnce(schedule_selector(Roulette::SetStopFlagTrue), 1.0f);
}

void Roulette::SetStopFlagTrue(float f) {
    Roulette::mb_RouletteStopFlag = true;
}


void Roulette::CreateMenu() {
    auto size_Visible = Director::getInstance()->getVisibleSize();
    auto vec_Origin = Director::getInstance()->getVisibleOrigin();
    
    {
        //0-1. 조작 메뉴 생성
        MenuItemImage* pImage_Replay = MenuItemImage::create( "res/CloseNormal.png", "res/CloseSelected.png", this,
                                                        menu_selector( Roulette::_StartSpin ) );
        MenuItemImage* pImage_Slow = MenuItemImage::create( "res/CloseNormal.png", "res/CloseSelected.png", this,
                                                        menu_selector( Roulette::_SlowSpin ) );
        MenuItemImage* pImage_SetFriction = MenuItemImage::create( "res/CloseNormal.png", "res/CloseSelected.png", this,
                                                        menu_selector( Roulette::_SetModestFriction ) );
        MenuItemImage* pImage_SetMaxFriction = MenuItemImage::create( "res/CloseNormal.png", "res/CloseSelected.png", this,
                                                        menu_selector( Roulette::_SetMaxFriction ) );
        MenuItemImage* pImage_Stop = MenuItemImage::create( "res/CloseNormal.png", "res/CloseSelected.png", this,
                                                        menu_selector( Roulette::_StopAbrupt ) );
        MenuItemImage* pImage_Reset = MenuItemImage::create( "res/CloseNormal.png", "res/CloseSelected.png", this,
                                                        menu_selector( Roulette::_InitObj ) );
        pImage_Replay->setTag( 1 );
        pImage_Slow->setTag( 2 );
        pImage_SetFriction->setTag( 3 );
        pImage_SetMaxFriction->setTag( 4 );
        pImage_Stop->setTag( 5 );
        pImage_Reset->setTag( 6 );
        
#if _REAL_MEASURE_
        pImage_Replay->setScale(2.0f);
        pImage_Slow->setScale(2.0f);
        pImage_SetFriction->setScale(2.0f);
        pImage_SetMaxFriction->setScale(2.0f);
        pImage_Stop->setScale(2.0f);
        pImage_Reset->setScale(2.0f);
#endif
        
        Menu* pMenu_BtnRight = Menu::create( pImage_Replay, pImage_Slow, pImage_SetFriction, pImage_SetMaxFriction, pImage_Stop, pImage_Reset, NULL );
        if ( !pMenu_BtnRight ) {
            return;
        }
        
        pMenu_BtnRight->setPosition(Point(size_Visible.width*4.0f/5.0f + vec_Origin.x,
                                     size_Visible.height/2.0f + vec_Origin.y));
        pMenu_BtnRight->alignItemsVertically();
        this->addChild( pMenu_BtnRight );
    }
    
    //0-2. 왼쪽에 랜덤 번호 픽 및 뽑기하는 버튼 생성
    {
        MenuItemImage* pImage_SetNumber = MenuItemImage::create( "res/CloseNormal.png", "res/CloseSelected.png", this,
                                                            menu_selector( Roulette::SpinRouletteWithNumberSelection ) );
        MenuItemImage* pImage_GetNumber = MenuItemImage::create( "res/CloseNormal.png", "res/CloseSelected.png", this,
                                                            menu_selector( Roulette::StopRouletteAtNumberSelected ) );
        
        
        pImage_SetNumber->setTag( 10 );
        pImage_GetNumber->setTag( 11 );
        
#if _REAL_MEASURE_
        pImage_SetNumber->setScale(2.0f);
        pImage_GetNumber->setScale(2.0f);
#endif
        
        Menu* pMenu_BtnLeft = Menu::create( pImage_SetNumber, pImage_GetNumber, NULL );
        if ( !pMenu_BtnLeft ) {
            return;
        }
        
        pMenu_BtnLeft->setPosition(Point(size_Visible.width*1.0f/5.0f + vec_Origin.x,
                                     size_Visible.height/2.0f + vec_Origin.y));
        pMenu_BtnLeft->alignItemsVertically();
        this->addChild( pMenu_BtnLeft );

    }
    
}

//룰렛을 만든다
void Roulette::SetObjectItem() {
    auto size_Visible = Director::getInstance()->getVisibleSize();
    auto vec_Origin = Director::getInstance()->getVisibleOrigin();
    
    //물리 효과 적용 범위 설정, PHYSICSBODY_MATERIAL_DEFAULT=밀도,반발계수,마찰계수 기본설정
    auto edgeBx_PhBody = PhysicsBody::createEdgeBox(size_Visible, PHYSICSBODY_MATERIAL_DEFAULT, 3);
    
    //물리 효과 적용 노드
    auto node_EdgeBx = Node::create();
    node_EdgeBx->setPosition(Point(size_Visible.width/2.0f + vec_Origin.x,
                                   size_Visible.height/2.0f + vec_Origin.y));
    node_EdgeBx->setPhysicsBody(edgeBx_PhBody);
    
    this->addChild(node_EdgeBx);
    
    cocos2d::Size size_Roulette;
    float f_RouletteSpriteScale;
    //1-1. 룰렛 원판 스프라이트 생성
    {
        auto sprite_RouletteBase = Sprite::create("res/circle.png");
        size_Roulette = sprite_RouletteBase->getContentSize(); //크기 값 외부로 전달
        if (sprite_RouletteBase == nullptr) {
            return;
        }
        
#if _REAL_MEASURE_
        f_RouletteSpriteScale = 0.5f;
        sprite_RouletteBase->setScale(f_RouletteSpriteScale);
        
#endif
        sprite_RouletteBase->setPosition(Point(size_Visible.width/2.0f + vec_Origin.x,
                                               size_Visible.height/2.0f + vec_Origin.y));
        
        //생성한 스프라이트에 물리 바디 입히기 PhysicsMaterial(밀도, 반발계수, 마찰계수)
        mphbody_RouletteBase = PhysicsBody::createCircle(sprite_RouletteBase->getContentSize().width/2.0f,
                                                         PhysicsMaterial(1.0f,1.0f,0.6f));
        //충돌이 일어나지 않게 설정(false)
        mphbody_RouletteBase->setCollisionBitmask(0);
        mphbody_RouletteBase->setContactTestBitmask(false);
        mphbody_RouletteBase->setGravityEnable(false);
        //출현시 동적 오브젝트로 설정
        mphbody_RouletteBase->setDynamic(true);
        sprite_RouletteBase->setPhysicsBody(mphbody_RouletteBase);
        
        this->addChild(sprite_RouletteBase);
        
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
        
    }
    
    
    //2-1. 룰렛 화살표 스프라이트 생성
    {
        auto sprite_RoulettePointer = Sprite::create("res/pointer.png");
        if (sprite_RoulettePointer == nullptr) {
            return;
        }
#if _REAL_MEASURE_
        float f_RoulettePointerSpriteScale = 0.1f;
        sprite_RoulettePointer->setScale(f_RoulettePointerSpriteScale);
        size_Roulette.setSize(size_Roulette.width * f_RouletteSpriteScale,
                              size_Roulette.height * f_RouletteSpriteScale);        //scale 조정된 값에 비례해서 커짐
#endif
        
        sprite_RoulettePointer->setPosition(Point(size_Visible.width/2 + vec_Origin.x,
                                                  size_Visible.height/2 + vec_Origin.y + size_Roulette.height*1/2));
        //        sprite_RoulettePointer->setAnchorPoint(Point(0.5f, 0.2f));
        
        
        //생성한 스프라이트에 물리 바디 입히기 PhysicsMaterial(밀도, 반발계수, 마찰계수)
        cocos2d::Size size_ArrowPhBox;
        size_ArrowPhBox.height = sprite_RoulettePointer->getContentSize().height; //*10.0f/9.0f;
        size_ArrowPhBox.width  = sprite_RoulettePointer->getContentSize().width/2.0f;
        auto phbody_RoulettePointer = PhysicsBody::createBox(size_ArrowPhBox,
                                                             PhysicsMaterial(0.3f,1.0f,1.0f));
        //        auto phbody_RoulettePointer = PhysicsBody::createCircle(sprite_RoulettePointer->getContentSize().width/2,
        //                                                              PhysicsMaterial(0,1,1));
        
//        PhysicsBody::create
        
        //출현시 속성 설정
        phbody_RoulettePointer->setGravityEnable(true);
        phbody_RoulettePointer->setMass(0.0f);
        phbody_RoulettePointer->setDynamic(true);
        phbody_RoulettePointer->setCollisionBitmask(ELEMENT_NUM + 1);   //8개의 요소인 경우 9번을 부여받는다
        phbody_RoulettePointer->setContactTestBitmask(true);
        
        sprite_RoulettePointer->setPhysicsBody(phbody_RoulettePointer);
        
        this->addChild(sprite_RoulettePointer);
        
        //핀포인트 설정
#if _REAL_MEASURE_
        auto vec_PinnedLocation = Point(sprite_RoulettePointer->getPosition().x,
                                        sprite_RoulettePointer->getPosition().y
                                        + sprite_RoulettePointer->getContentSize().height*f_RoulettePointerSpriteScale/3);
#else
        auto vec_PinnedLocation = Point(sprite_RoulettePointer->getPosition().x,
                                        sprite_RoulettePointer->getPosition().y
                                        + sprite_RoulettePointer->getContentSize().height/3);
#endif
        auto pin_StaticPointer = PhysicsJointPin::construct(phbody_RoulettePointer, edgeBx_PhBody, vec_PinnedLocation);
        //        auto pin_StaticPointer = PhysicsJointFixed::construct(phbody_RoulettePointer, edgeBx_PhBody, Point(size_Visible.width/2,
        //                                                                                                           size_Visible.height/2 + 50));
        //        auto pin_StaticPointer = PhysicsJointRotarySpring::construct(phbody_RoulettePointer, edgeBx_PhBody, 200.0f, 1.0f); // sprite_RoulettePointer->getPosition(), edgeBx_PhBody->getPosition());
        //        pin_StaticPointer->createConstraints();
        this->getScene()->getPhysicsWorld()->addJoint(pin_StaticPointer);
        

        //2-2. 포인터 움직임 조정해줄 physicsbody 생성
        auto sprite_PointerLimit = Sprite::create("res/button.png");
        sprite_PointerLimit->setVisible(false);
#if _REAL_MEASURE_
        float f_LimitSpriteScale = 0.05f;
        sprite_PointerLimit->setScale(f_LimitSpriteScale);
#endif

        //룰렛 포인터를 기준으로 포지션 설정
#if _REAL_MEASURE_
        sprite_PointerLimit->setPosition(Point( sprite_RoulettePointer->getPosition().x
                                               + size_ArrowPhBox.width*f_RoulettePointerSpriteScale,
                                                sprite_RoulettePointer->getPosition().y
                                               + sprite_RoulettePointer->getContentSize().height*f_RoulettePointerSpriteScale/2.0f ) );
        //PhysicsMaterial(밀도, 반발계수, 마찰계수)
        auto phbody_PointerLimit =  PhysicsBody::createCircle(sprite_PointerLimit->getContentSize().width/2.0f,
                                                              PhysicsMaterial(1.0f,0.0f,1.0f));
#else
        sprite_PointerLimit->setPosition(Point( sprite_RoulettePointer->getPosition().x + size_ArrowPhBox.width,
                                               sprite_RoulettePointer->getPosition().y
                                               + sprite_RoulettePointer->getContentSize().height/2.0f ) );

        auto phbody_PointerLimit =  PhysicsBody::createCircle(sprite_PointerLimit->getContentSize().width/2.0f,
                                                              PhysicsMaterial(1.0f,0.0f,1.0f));
#endif

        phbody_PointerLimit->setDynamic(false);
        //        phbody_PointerLimit->setMass(10.0f);
        sprite_PointerLimit->setPhysicsBody(phbody_PointerLimit);

        this->addChild(sprite_PointerLimit);
    }
}

//3전 영역부터 속도를 줄인다
int Roulette::SlowEffectStartNo(int nTargetNo) {
    if (ELEMENT_NUM < 4) return 0;
    if (nTargetNo > 3) {
        return nTargetNo - 3;
    } else {
        return nTargetNo - 3 + ELEMENT_NUM;
    }
}


bool Roulette::onContactBegin(cocos2d::PhysicsContact &contact) {
    PhysicsBody* phbody_A = contact.getShapeA()->getBody();
    PhysicsBody* phbody_B = contact.getShapeB()->getBody();
    int n_BitmaskFmPointer = ELEMENT_NUM+1;
    
    //원판의 화살표 스프라이트와 각각의 노드가 닿았는지 조사
    for(int i = 0; i<ELEMENT_NUM; i++) {
        int n_BitmaskFmDots = i+1;
        if ((phbody_A->getCollisionBitmask() == n_BitmaskFmPointer && phbody_B->getCollisionBitmask() == n_BitmaskFmDots) ||
            (phbody_A->getCollisionBitmask() == n_BitmaskFmDots && phbody_B->getCollisionBitmask() == n_BitmaskFmPointer)) {
            
            if (phbody_A->getAngularVelocity() == 0.0f && phbody_B->getAngularVelocity() == 0.0f) {
                mphbody_RouletteBase->setAngularVelocity(1.0f);
                mphbody_RouletteBase->setAngularDamping(1.0f);
#if _DEBUG_
                log("dot touched: %d", n_BitmaskFmDots);
#endif
            }
        }
    }
    return true;
}


bool Roulette::onContactSeperate(cocos2d::PhysicsContact &contact, int *nTargetNo, bool *bOnSelected) {
    PhysicsBody* phbody_A = contact.getShapeA()->getBody();
    PhysicsBody* phbody_B = contact.getShapeB()->getBody();
    int n_BitmaskFmPointer = ELEMENT_NUM+1;
    
    //원판의 화살표 스프라이트와 각각의 노드가 떨어졌는지 조사
    for(int i = 0; i<ELEMENT_NUM; i++) {
        int n_BitmaskFmDots = i+1;
        if ((phbody_A->getCollisionBitmask() == n_BitmaskFmPointer && phbody_B->getCollisionBitmask() == n_BitmaskFmDots) ||
            (phbody_A->getCollisionBitmask() == n_BitmaskFmDots && phbody_B->getCollisionBitmask() == n_BitmaskFmPointer)) {
            if ( *nTargetNo == n_BitmaskFmDots && *bOnSelected == true ){
#if _DEBUG_
                if( ELEMENT_NUM == 8 ) {
                    std::vector<int> vn_Item = { 0, 1000, 0, 10, 20, 30, 50, 100, 500 };
                    log("final dot touched: %d => %d points", n_BitmaskFmDots, vn_Item.at(n_BitmaskFmDots));
                } else {
                    log("final dot touched: %d", n_BitmaskFmDots);
                }
#endif
                mphbody_RouletteBase->setAngularDamping(3.0f);
                mb_RouletteStopFlag = false;
                break;
            }
            if ( SlowEffectStartNo(*nTargetNo) == n_BitmaskFmDots && *bOnSelected == true ) {
                mphbody_RouletteBase->setAngularDamping(0.15f);
            }
        }
    }
    return true;
}


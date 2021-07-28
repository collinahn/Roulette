#ifndef __ROULETTE_H__
#define __ROULETTE_H__

#include "cocos2d.h"
#include "cocos-ext.h"
#include "ui/CocosGUI.h"

#define PI 3.14159265359
#define ELEMENT_NUM 8           //룰렛 요소의 갯수를 정한다

#define _VIS_DEBUG_ 0           //충돌구역 시각화
#define _DEBUG_ 1               //로그 활성화 set : 1
#define _REAL_MEASURE_ 1        //스프라이트 크기에 맞는 리소스로 변경할 경우 set 0


class Roulette : public cocos2d::Scene {
public:
    static cocos2d::Scene* createScene();

    virtual bool init();
    
    void reOrderSprite(cocos2d::Sprite *pSenderSprite);
    virtual void onEnter();
    virtual void onExit();
        
    CREATE_FUNC(Roulette);
    
private:
//    cocos2d::Sprite * msprite_RestartBtn;
    cocos2d::PhysicsWorld * sceneWorld;
    cocos2d::PhysicsBody * mphbody_RouletteBase;
    
    int mn_TargetRandNum = 0;           // 1~ELEMENT_NUM
    bool mb_RouletteStopFlag = false;           //
    void SetStopFlagTrue(float f);

    void CreateMenu();
    void SetObjectItem();

    void SpinRouletteWithNumberSelection(cocos2d::CCObject* pSender);
    void StopRouletteAtNumberSelected(cocos2d::CCObject* pSender);
    void _StartSpin(cocos2d::CCObject* pSender);
    void _SlowSpin(cocos2d::CCObject* pSender);
    void _SetModestFriction(cocos2d::CCObject* pSender);
    void _SetMaxFriction(cocos2d::CCObject* pSender);
    void _StopAbrupt(cocos2d::CCObject* pSender);
    void _InitObj(cocos2d::CCObject* pSender);
    void SpinRoulette();
    void SetPhysicsWorld(cocos2d::PhysicsWorld *world) { sceneWorld = world; };
    
    int SlowEffectStartNo(int nTargetNo);
    bool onContactBegin(cocos2d::PhysicsContact &contact); //원판과 화살표가 닿았을 때 화살표의 변형 여부 결정
    bool onContactSeperate(cocos2d::PhysicsContact &contact, int *nTargetNo, bool *bOnSelected); //원판과 화살표가 떨어졌을 때 화살표의 변형 여부 결정
};

#endif // __ROULETTE_H__

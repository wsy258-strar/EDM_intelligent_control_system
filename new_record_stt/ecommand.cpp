#include<iostream>
#include <QtWidgets/QMainWindow>
#include"ecommand.h"

using namespace std;

ON::ON(int a) :on_value(a){}
void ON::setON(int value) {

    if (((on_value + value) >= 0 && (on_value + value) <= 63) || ((on_value + value) >= 100 && (on_value + value) <= 107)) {
        on_value += value;
    }
    else {
        qDebug() << QString("设定参数超过范围，请重新发布指令！") ;
    }

}

int ON::getON() {

    return on_value;

}

OFF::OFF(double a) :off_value(a) {}
void OFF::setOFF(double value) {

    if ((off_value + value) >= 0 && (off_value + value) <= 63.0) {
        off_value += value;
    }
    else {
        qDebug() << QString("设定参数超过范围，请重新发布指令！" );
    }


}

double OFF::getOFF() {

    return off_value;

}

IP::IP(int a) :ip_value(a) {}
void IP::setIP(int value) {

    ip_value = value;

}

int IP::getIP() {

    return ip_value;

}

void PL::setPL(int value) {

    pl_value = value;

}

int PL::getPL() {

    return pl_value;

}

V::V(int a) :v_value(a) {}
void V::setV(int value) {

    v_value = value;

}

int V::getV() {

    return v_value;

}

void HP::setHP(int value) {

    hp_value = value;

}

int HP::getHP() {

    return hp_value;

}

void PP::setPP(int value) {

    pp_value = value;

}

int PP::getPP() {

    return pp_value;

}

void AL::setAL(int value) {

    al_value = value;

}

int AL::getAL() {

    return al_value;

}

void OC::setOC(int value) {

    oc_value = value;

}

int OC::getOC() {

    return oc_value;

}

void LD::setLD(int value) {

    ld_value = value;

}

int LD::getLD() {

    return ld_value;

}

void MU::setMU(int value) {

    mu_value = value;

}

MU::MU(int a) :mu_value(a) {}
int MU::getMU() {

    return mu_value;

}

void GAP::setGAP(int value) {

    gap_value = value;

}

int GAP::getGAP() {

    return gap_value;

}

void UP::setUP(int value) {

    up_value = value;

}

int UP::getUP() {

    return up_value;

}

void DN::setDN(int value) {

    dn_value = value;

}

int DN::getDN() {

    return dn_value;

}

void CA::setCA(int value) {

    ca_value = value;

}

int CA::getCA() {

    return ca_value;

}

void S::setS(int value) {

    s_value = value;

}

int S::getS() {

    return s_value;

}

void LN::setLN(int value) {

    ln_value = value;

}

int LN::getLN() {

    return ln_value;

}

void STEP::setSTEP(int value) {

    step_value = value;

}

int STEP::getSTEP() {

    return step_value;

}

void L::setL(int value) {

    l_value = value;

}

int L::getL() {

    return l_value;

}

//void LP::setLP(int value) {
//
//    lp_value = value;
//
//}
//
//int LP::getLP() {
//
//    return lp_value;
//
//}
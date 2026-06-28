/**
 * @file    ecommand.cpp
 * @brief   电火花加工（EDM）参数实体类实现
 *
 * 本文件实现了 ecommand.h 中声明的 20 个 EDM 参数类的构造函数、
 * setter 和 getter 方法。每个类遵循统一的三段式实现模式：
 * - 构造函数：通过初始化列表设置成员变量初值
 * - setter：修改成员变量值
 * - getter：返回成员变量当前值
 *
 * @note MyLP 类的实现为内联（见 ecommand.h），不在此文件中。
 */

#include "ecommand.h"

/* ======================== ON — 脉冲放电时间 ======================== */
ON::ON(int a) : on_value(a) {}

void ON::setON(int value) {
    on_value = value;
}

int ON::getON() {
    return on_value;
}

/* ======================== OFF — 脉冲休止时间 ======================== */
OFF::OFF(int a) : off_value(a) {}

void OFF::setOFF(int value) {
    off_value = value;
}

int OFF::getOFF() {
    return off_value;
}

/* ======================== IP — 放电电流峰值 ======================== */
IP::IP(double a) : ip_value(a) {}

void IP::setIP(double value) {
    ip_value = value;
}

double IP::getIP() {
    return ip_value;
}

/* ======================== PL — 放电极性 ======================== */
PL::PL(char a) : pl_value(a) {}

void PL::setPL(char value) {
    pl_value = value;
}

char PL::getPL() {
    return pl_value;
}

/* ======================== V — 直流电压 ======================== */
V::V(int a) : v_value(a) {}

void V::setV(int value) {
    v_value = value;
}

int V::getV() {
    return v_value;
}

/* ======================== HP — NOW回路/高压辅助控制 ======================== */
HP::HP(int a) : hp_value(a) {}

void HP::setHP(int value) {
    hp_value = value;
}

int HP::getHP() {
    return hp_value;
}

/* ======================== PP — PIKADEN脉冲控制 ======================== */
PP::PP(std::string a) : pp_value(a) {}

void PP::setPP(std::string value) {
    pp_value = value;
}

std::string PP::getPP() {
    return pp_value;
}

/* ======================== AL — 异常放电检验标准 ======================== */
AL::AL(int a) : al_value(a) {}

void AL::setAL(int value) {
    al_value = value;
}

int AL::getAL() {
    return al_value;
}

/* ======================== OC — 预留参数 ======================== */
OC::OC(int a) : oc_value(a) {}

void OC::setOC(int value) {
    oc_value = value;
}

int OC::getOC() {
    return oc_value;
}

/* ======================== LD — 预留参数 ======================== */
LD::LD(int a) : ld_value(a) {}

void LD::setLD(int value) {
    ld_value = value;
}

int LD::getLD() {
    return ld_value;
}

/* ======================== MU — 脉冲幅度放大倍率 ======================== */
MU::MU(int a) : mu_value(a) {}

void MU::setMU(int value) {
    mu_value = value;
}

int MU::getMU() {
    return mu_value;
}

/* ======================== GAP — 伺服基准电压 ======================== */
GAP::GAP(int a) : gap_value(a) {}

void GAP::setGAP(int value) {
    gap_value = value;
}

int GAP::getGAP() {
    return gap_value;
}

/* ======================== UP — 自动抬刀抬升时间 ======================== */
UP::UP(int a) : up_value(a) {}

void UP::setUP(int value) {
    up_value = value;
}

int UP::getUP() {
    return up_value;
}

/* ======================== DN — 自动抬刀下降时间 ======================== */
DN::DN(int a) : dn_value(a) {}

void DN::setDN(int value) {
    dn_value = value;
}

int DN::getDN() {
    return dn_value;
}

/* ======================== CA — 极间电容器容量 ======================== */
CA::CA(int a) : ca_value(a) {}

void CA::setCA(int value) {
    ca_value = value;
}

int CA::getCA() {
    return ca_value;
}

/* ======================== S — 伺服速度 ======================== */
S::S(int a) : s_value(a) {}

void S::setS(int value) {
    s_value = value;
}

int S::getS() {
    return s_value;
}

/* ======================== LN — 预留参数 ======================== */
LN::LN(int a) : ln_value(a) {}

void LN::setLN(int value) {
    ln_value = value;
}

int LN::getLN() {
    return ln_value;
}

/* ======================== STEP — 摇动半径 ======================== */
STEP::STEP(int a) : step_value(a) {}

void STEP::setSTEP(int value) {
    step_value = value;
}

int STEP::getSTEP() {
    return step_value;
}

/* ======================== L — 预留参数 ======================== */
L::L(int a) : l_value(a) {}

void L::setL(int value) {
    l_value = value;
}

int L::getL() {
    return l_value;
}

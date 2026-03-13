#include "ecommand.h"  // 包含头文件

// ==================== ON类 实现 ====================
// 构造函数：初始化on_value
ON::ON(int a) : on_value(a) {}

// 设置on_value的值
void ON::setON(int value) {
    on_value = value;
}

// 获取on_value的值
int ON::getON() {
    return on_value;
}

// ==================== OFF类 实现 ====================
OFF::OFF(int a) : off_value(a) {}

void OFF::setOFF(int value) {
    off_value = value;
}

int OFF::getOFF() {
    return off_value;
}

// ==================== IP类 实现 ====================
IP::IP(double a) : ip_value(a) {}

void IP::setIP(double value) {
    ip_value = value;
}

double IP::getIP() {
    return ip_value;
}

// ==================== PL类 实现 ====================
PL::PL(char a) : pl_value(a) {}

void PL::setPL(char value) {
    pl_value = value;
}

char PL::getPL() {
    return pl_value;
}

// ==================== V类 实现 ====================
V::V(int a) : v_value(a) {}

void V::setV(int value) {
    v_value = value;
}

int V::getV() {
    return v_value;
}

// ==================== HP类 实现 ====================
HP::HP(int a) : hp_value(a) {}

void HP::setHP(int value) {
    hp_value = value;
}

int HP::getHP() {
    return hp_value;
}

// ==================== PP类 实现 ====================
PP::PP(int a) : pp_value(a) {}

void PP::setPP(int value) {
    pp_value = value;
}

int PP::getPP() {
    return pp_value;
}
std::string PP::getPPDisplay() {
    switch (pp_value) {
    case 0: return "00";
    case 1: return "01";
    case 2: return "10";
    case 3: return "11";
    default: return "无效值";
    }
}

// ==================== AL类 实现 ====================
AL::AL(int a) : al_value(a) {}

void AL::setAL(int value) {
    al_value = value;
}

int AL::getAL() {
    return al_value;
}

// ==================== OC类 实现 ====================
OC::OC(int a) : oc_value(a) {}

void OC::setOC(int value) {
    oc_value = value;
}

int OC::getOC() {
    return oc_value;
}

// ==================== LD类 实现 ====================
LD::LD(int a) : ld_value(a) {}

void LD::setLD(int value) {
    ld_value = value;
}

int LD::getLD() {
    return ld_value;
}

// ==================== MU类 实现 ====================
MU::MU(int a) : mu_value(a) {}

void MU::setMU(int value) {
    mu_value = value;
}

int MU::getMU() {
    return mu_value;
}

// ==================== GAP类 实现 ====================
GAP::GAP(int a) : gap_value(a) {}

void GAP::setGAP(int value) {
    gap_value = value;
}

int GAP::getGAP() {
    return gap_value;
}

// ==================== UP类 实现 ====================
UP::UP(int a) : up_value(a) {}

void UP::setUP(int value) {
    up_value = value;
}

int UP::getUP() {
    return up_value;
}

// ==================== DN类 实现 ====================
DN::DN(int a) : dn_value(a) {}

void DN::setDN(int value) {
    dn_value = value;
}

int DN::getDN() {
    return dn_value;
}

// ==================== CA类 实现 ====================
CA::CA(int a) : ca_value(a) {}

void CA::setCA(int value) {
    ca_value = value;
}

int CA::getCA() {
    return ca_value;
}

// ==================== S类 实现 ====================
S::S(int a) : s_value(a) {}

void S::setS(int value) {
    s_value = value;
}

int S::getS() {
    return s_value;
}

// ==================== LN类 实现 ====================
LN::LN(int a) : ln_value(a) {}

void LN::setLN(int value) {
    ln_value = value;
}

int LN::getLN() {
    return ln_value;
}

// ==================== STEP类 实现 ====================
STEP::STEP(int a) : step_value(a) {}

void STEP::setSTEP(int value) {
    step_value = value;
}

int STEP::getSTEP() {
    return step_value;
}

// ==================== L类 实现 ====================
L::L(int a) : l_value(a) {}

void L::setL(int value) {
    l_value = value;
}

int L::getL() {
    return l_value;
}

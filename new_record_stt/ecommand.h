#include<iostream>
class ON
{
public:
    ON(int a);
    int on_value;
    void setON(int value);
    int getON();
};

class OFF
{
public:
    OFF(int a);
    int off_value;
    void setOFF(int value);
    int getOFF();
};

class IP
{
public:
    IP(double a);
    double ip_value;
    void setIP(double value);
    double getIP();
};

class PL
{
public:
    PL(char a);
    char pl_value;
    void setPL(char value);
    char getPL();
};

class V
{
public:
    V(int a);
    int v_value;
    void setV(int value);
    int getV();
};

class HP
{
public:
    HP(int a);
    int hp_value;
    void setHP(int value);
    int getHP();
};

class PP
{
public:
    PP(int a);
    int pp_value;
    void setPP(int value);
    int getPP();
    // 新增：返回显示用的字符串（00/01/10/11）
    std::string getPPDisplay();
};

class AL
{
public:
    AL(int a);
    int al_value;
    void setAL(int value);
    int getAL();
};

class OC
{
public:
    OC(int a);
    int oc_value;
    void setOC(int value);
    int getOC();
};

class LD
{
public:
    LD(int a);
    int ld_value;
    void setLD(int value);
    int getLD();
};

class MU
{
public:
    MU(int a);
    int mu_value;
    void setMU(int value);
    int getMU();
};

class GAP
{
public:
    GAP(int a);
    int gap_value;
    void setGAP(int value);
    int getGAP();
};

class UP
{
public:
    UP(int a);
    int up_value;
    void setUP(int value);
    int getUP();
};

class DN
{
public:
    DN(int a);
    int dn_value;
    void setDN(int value);
    int getDN();
};

class CA
{
public:
    CA(int a);
    int ca_value;
    void setCA(int value);
    int getCA();
};

class S
{
public:
    S(int a);
    int s_value;
    void setS(int value);
    int getS();
};

class LN
{
public:
    LN(int a);
    int ln_value;
    void setLN(int value);
    int getLN();
};

class STEP
{
public:
    STEP(int a);
    int step_value;
    void setSTEP(int value);
    int getSTEP();
};

class L
{
public:
    L(int a);
    int l_value;
    void setL(int value);
    int getL();
};

class MyLP
{
public:
    MyLP(int a) : lp_value(a){};
    void setLP(int value)
    {
        lp_value = value; // 将传入的value赋值给成员变量
    }
    // 类内实现getLP函数：返回lp_value的值
    int getLP()
    {
        return lp_value; // 返回成员变量的值
    }
    int lp_value; // 成员变量
};
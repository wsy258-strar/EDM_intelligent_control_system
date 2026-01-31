
#ifndef ECOMMAND_H
#define ECOMMAND_H

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
    OFF(double a);
    double off_value;
    void setOFF(double value);
    double getOFF();
};

class IP
{
public:
    IP(int a);
    int ip_value;
    void setIP(int value);
    int getIP();
};

class PL
{
public:
    int pl_value;
    void setPL(int value);
    int getPL();
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
    int hp_value;
    void setHP(int value);
    int getHP();
};

class PP
{
public:
    int pp_value;
    void setPP(int value);
    int getPP();
};

class AL
{
public:
    int al_value;
    void setAL(int value);
    int getAL();
};

class OC
{
public:
    int oc_value;
    void setOC(int value);
    int getOC();
};

class LD
{
public:
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
    int gap_value;
    void setGAP(int value);
    int getGAP();
};

class UP
{
public:
    int up_value;
    void setUP(int value);
    int getUP();
};

class DN
{
public:
    int dn_value;
    void setDN(int value);
    int getDN();
};

class CA
{
public:
    int ca_value;
    void setCA(int value);
    int getCA();
};

class S
{
public:
    int s_value;
    void setS(int value);
    int getS();
};

class LN
{
public:
    int ln_value;
    void setLN(int value);
    int getLN();
};

class STEP
{
public:
    int step_value;
    void setSTEP(int value);
    int getSTEP();
};

class L
{
public:
    int l_value;
    void setL(int value);
    int getL();
};

//class LP
//{
//public:
//    int lp_value;
//    void setLP(int value);
//    int getLP();
//};
#endif // ECOMMAND_H
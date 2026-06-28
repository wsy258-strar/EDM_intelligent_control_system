/**
 * @file    ecommand.h
 * @brief   电火花加工（EDM）参数实体类声明
 *
 * 本文件声明了电火花加工各参数的封装类，每个类对应一个加工参数，
 * 提供参数值存储（getter）与修改（setter）的接口。
 *
 * 设计模式：每个 EDM 参数封装为独立的类，遵循单一职责原则。
 * 各类型根据参数数据类型分别以 int、double、char 或 std::string 进行存储。
 *
 * 参数列表（共 20 个）：
 *   ON    - 脉冲放电时间         | 类型 int    | 范围 0~63, 100~107
 *   OFF   - 脉冲休止时间         | 类型 int    | 范围 0~63
 *   IP    - 放电电流峰值         | 类型 double | 最小单位 0.5
 *   PL    - 放电极性             | 类型 char   | '+' 或 '-'
 *   V     - 直流电压             | 类型 int    | 01 或 02
 *   HP    - NOW回路/高压辅助控制  | 类型 int    | 两位数值
 *   PP    - PIKADEN脉冲控制      | 类型 string | "00"/"01"/"10"/"11"
 *   AL    - 异常放电检验标准     | 类型 int    | 0~63（基准 33）
 *   OC    - 预留参数             | 类型 int    | 预留
 *   LD    - 预留参数             | 类型 int    | 预留
 *   MU    - 脉冲幅度放大倍率     | 类型 int    | 0~9，对应 ×1~×10
 *   GAP   - 伺服基准电压         | 类型 int    | 0~9，对应 0V~130V
 *   UP    - 自动抬刀抬升时间     | 类型 int    | 0（无跳刀）或 1~9
 *   DN    - 自动抬刀下降时间     | 类型 int    | 配合 UP 使用
 *   CA    - 极间电容器容量       | 类型 int    | 0~9，对应 0~1.4μF
 *   S     - 伺服速度             | 类型 int    | 0~9，从快到慢
 *   LN    - 预留参数             | 类型 int    | 预留
 *   STEP  - 摇动半径（放电间隔） | 类型 int    | 0~99999 μm
 *   L     - 预留参数             | 类型 int    | 预留
 *   MyLP  - 预留参数             | 类型 int    | 预留
 */

#include <iostream>

/**
 * @class ON
 * @brief 脉冲放电时间参数
 *
 * 设定 1 个脉冲的放电时间（Pulse ON time），
 * 设定范围：0~63 及 100~107。
 * 放电时间与加工速度、电极消耗、表面粗糙度密切相关。
 */
class ON
{
public:
    explicit ON(int a);
    void setON(int value);
    int getON();

private:
    int on_value;   ///< 脉冲放电时间设定值
};

/**
 * @class OFF
 * @brief 脉冲放点后休止时间参数
 *
 * 设定 1 个脉冲放电后的休止时间（Pulse OFF time），
 * 设定范围：0~63。
 * OFF 时间影响排屑效果和加工稳定性。
 */
class OFF
{
public:
    explicit OFF(int a);
    void setOFF(int value);
    int getOFF();

private:
    int off_value;  ///< 脉冲休止时间设定值
};

/**
 * @class IP
 * @brief 放电电流峰值参数
 *
 * 设定 1 个脉冲的放电电流峰值（Peak Current），
 * 与放电时间 ON 组合是决定加工速度、表面粗糙度、
 * 电极消耗及放电加工性能的最重要参数。
 * IP 最小输入单位为 0.5，值必须为 0.5 的整数倍。
 */
class IP
{
public:
    explicit IP(double a);
    void setIP(double value);
    double getIP();

private:
    double ip_value;    ///< 放电电流峰值（单位：A）
};

/**
 * @class PL
 * @brief 放电极性参数
 *
 * 选择电极与工件的放电极性，以主轴侧（通常为电极）为基准：
 * - 正极性加工：工件为 '+'，电极为 '-'
 * - 负极性加工：工件为 '-'，电极为 '+'
 */
class PL
{
public:
    explicit PL(char a);
    void setPL(char value);
    char getPL();

private:
    char pl_value;      ///< 极性子符（'+' 正极性 / '-' 负极性）
};

/**
 * @class V
 * @brief 直流电压参数
 *
 * 用于转换提供 IP 电流的直流电压：
 * - 01：对应 Cu-ST 加工（90VDC）
 * - 02：对应 Gr-ST / CuW-W 加工（120VDC）
 */
class V
{
public:
    explicit V(int a);
    void setV(int value);
    int getV();

private:
    int v_value;        ///< 直流电压档位（01 或 02）
};

/**
 * @class HP
 * @brief NOW回路与高压辅助回路控制参数
 *
 * 用于控制降低电极消耗的 NOW 回路、电压控制回路（低压/中压）
 * 和高压辅助回路（ON/OFF 及电流追加）。
 *
 * 设定值为两位数值：
 * - 十位：控制电压回路和 NOW 回路状态
 *   （00=低压+NOW OFF, 40=低压+NOW ON, 10=中压+NOW OFF 等）
 * - 个位（0~7）：控制高压辅助电流
 *   （0=无电流, 1~7 依次对应 0.5A~3.5A，每档递增 0.5A）
 *
 * 无负荷电压基准值由十位决定，可选 90V/120V/150V/280V。
 */
class HP
{
public:
    explicit HP(int a);
    void setHP(int value);
    int getHP();

private:
    int hp_value;       ///< HP 控制值（两位数值）
};

/**
 * @class PP
 * @brief PIKADEN 脉冲控制参数
 *
 * 设定 PIKADEN 脉冲控制模式：
 * - "00"：高压 + PIKADEN OFF
 * - "01"：仅 PIKADEN ON
 * - "10"：仅高压 ON
 * - "11"：高压 + PIKADEN ON
 */
class PP
{
public:
    explicit PP(std::string a);
    void setPP(std::string value);
    std::string getPP();

private:
    std::string pp_value;   ///< PIKADEN 控制字符串
};

/**
 * @class AL
 * @brief 异常放电检验标准参数
 *
 * 设定范围：0~63（基准值为 33）。
 * 设定值越大，加工速度越快，但耐电弧性随之降低。
 * 用于控制电极间异常放电检测的灵敏度。
 */
class AL
{
public:
    explicit AL(int a);
    void setAL(int value);
    int getAL();

private:
    int al_value;       ///< 异常放电检验标准设定值
};

/**
 * @class OC
 * @brief 预留参数 OC
 *
 * 预留给未来扩展使用的参数，当前无实际功能。
 */
class OC
{
public:
    explicit OC(int a);
    void setOC(int value);
    int getOC();

private:
    int oc_value;       ///< OC 预留参数值
};

/**
 * @class LD
 * @brief 预留参数 LD
 *
 * 预留给未来扩展使用的参数，当前无实际功能。
 */
class LD
{
public:
    explicit LD(int a);
    void setLD(int value);
    int getLD();

private:
    int ld_value;       ///< LD 预留参数值
};

/**
 * @class MU
 * @brief 脉冲幅度放大倍率参数
 *
 * 设定单次脉冲放电后休止时间 OFF 阶段的脉冲幅度放大倍率。
 * 设定值 0~9 分别对应 ×1 到 ×10 的放大倍率。
 * 用于控制加工过程中的脉冲能量调制。
 */
class MU
{
public:
    explicit MU(int a);
    void setMU(int value);
    int getMU();

private:
    int mu_value;       ///< 脉冲幅度放大倍率档位
};

/**
 * @class GAP
 * @brief 伺服基准电压参数
 *
 * 设定伺服控制系统的基准电压，用于控制加工间隙。
 * 设定值 0~9 的对应关系：
 * 0→0V, 1→15V, 2→25V, 3→35V, 4→45V,
 * 5→60V, 6→70V, 7→80V, 8→120V, 9→130V
 * （实际电压值随加工条件略有差异）
 */
class GAP
{
public:
    explicit GAP(int a);
    void setGAP(int value);
    int getGAP();

private:
    int gap_value;      ///< 伺服基准电压档位（0~9）
};

/**
 * @class UP
 * @brief 自动抬刀抬升时间参数
 *
 * 控制自动抬刀的抬升时间：
 * - 设定为 0：无跳刀动作
 * - 设定为 1~9：配合 DN 参数启用自动抬刀功能
 *
 * 自动抬刀用于排出加工屑，提高加工稳定性。
 */
class UP
{
public:
    explicit UP(int a);
    void setUP(int value);
    int getUP();

private:
    int up_value;       ///< 抬刀抬升时间设定值
};

/**
 * @class DN
 * @brief 自动抬刀下降时间参数
 *
 * 控制自动抬刀的下降时间，与 UP 组成一个完整的抬刀周期。
 * 仅当 UP 设定为 1~9 时生效。
 * 粗加工/精加工需按 DN = UP + 固定差值的规则设置。
 */
class DN
{
public:
    explicit DN(int a);
    void setDN(int value);
    int getDN();

private:
    int dn_value;       ///< 抬刀下降时间设定值
};

/**
 * @class CA
 * @brief 极间电容器容量参数
 *
 * 设定极间电容器的容量，用于控制放电能量。
 * 设定值 0~9 对应关系：
 * 0→0μF, 1→0.006μF, 2→0.012μF, 3→0.025μF, 4→0.05μF,
 * 5→0.1μF, 6→0.2μF, 7→0.4μF, 8→0.8μF, 9→1.4μF
 *
 * 适用于有消耗条件下的精加工、细孔加工及电极成形加工。
 */
class CA
{
public:
    explicit CA(int a);
    void setCA(int value);
    int getCA();

private:
    int ca_value;       ///< 极间电容器容量档位（0~9）
};

/**
 * @class S
 * @brief 伺服速度参数
 *
 * 设定伺服电机的进给速度。
 * 设定范围：0~9，对应伺服速度从快到慢。
 * 通常设为 2 或 3，在细孔、电极成形等加工时需调高设定值以避免伺服轴振动。
 */
class S
{
public:
    explicit S(int a);
    void setS(int value);
    int getS();

private:
    int s_value;        ///< 伺服速度档位（0~9）
};

/**
 * @class LN
 * @brief 预留参数 LN
 *
 * 预留给未来扩展使用的参数，当前无实际功能。
 */
class LN
{
public:
    explicit LN(int a);
    void setLN(int value);
    int getLN();

private:
    int ln_value;       ///< LN 预留参数值
};

/**
 * @class STEP
 * @brief 摇动半径（放电间隔）参数
 *
 * 用于设定摇动加工的摇动半径（即放电间隔）。
 * 设定范围：0~99999，单位为 μm。
 * 摇动加工用于扩大加工范围、改善排屑和加工精度。
 */
class STEP
{
public:
    explicit STEP(int a);
    void setSTEP(int value);
    int getSTEP();

private:
    int step_value;     ///< 摇动半径值（单位：μm）
};

/**
 * @class L
 * @brief 预留参数 L
 *
 * 预留给未来扩展使用的参数，当前无实际功能。
 */
class L
{
public:
    explicit L(int a);
    void setL(int value);
    int getL();

private:
    int l_value;        ///< L 预留参数值
};

/**
 * @class MyLP
 * @brief 预留参数 LP
 *
 * 预留给未来扩展使用的参数，当前无实际功能。
 * 此类的所有方法均为内联实现。
 */
class MyLP
{
public:
    explicit MyLP(int a) : lp_value(a) {}
    void setLP(int value) { lp_value = value; }
    int getLP() { return lp_value; }

private:
    int lp_value;       ///< LP 预留参数值
};

#ifndef _SWITCH_H_
#define _SWITCH_H_

/**
 * チャタリング対策入れた Switchクラス
 * INPUTはプルダウン
*/
class Switch
{
    private:
    unsigned char swFlag;
    unsigned char lastSW, SW;
    unsigned char chatt[3];

    public:
    Switch();

    inline unsigned char getSWFlag() { return swFlag; }

    /**
     * readValue : digitalReadで読み取った値
    */
    void updateState(unsigned char readValue);

    void resetFlag();
};

#endif
//
// Created by TIAN FENG on 2019/8/13.
//

#ifndef NATIVEVIDEO_PLAYERSTATUS_H
#define NATIVEVIDEO_PLAYERSTATUS_H

/**
 * 是否退出，打算用这个变量来做退出(销毁)
 */

class PlayerStatus {
public:
    bool isExit;

    PlayerStatus();

    ~PlayerStatus();
};

#endif //NATIVEVIDEO_PLAYERSTATUS_H


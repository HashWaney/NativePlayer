/**
 * Created by Hash on 2020-04-14.
 */


#ifndef AUDIOPLAYER_PLAYSTATUS_H
#define AUDIOPLAYER_PLAYSTATUS_H


class PlayStatus {

public:
    bool exit = false;
    bool seekByUser = false;
public:
    PlayStatus();

    ~PlayStatus();

};


#endif //AUDIOPLAYER_PLAYSTATUS_H

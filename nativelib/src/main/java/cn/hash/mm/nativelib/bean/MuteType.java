package cn.hash.mm.nativelib.bean;

/**
 * Created by Hash on 2020-04-16.
 */


public enum MuteType {

    MUTE_TYPE_RIGHT("RIGHT_VOLUME", 0),
    MUTE_TYPE_LEFT("LEFT_VOLUME", 1),
    MUTE_TYPE_CENTER("CENTER_VOLUME", 2);

    private String name;
    private int value;

    MuteType(String name, int value) {
        this.name = name;
        this.value = value;

    }

    public int getValue() {
        return value;
    }
}

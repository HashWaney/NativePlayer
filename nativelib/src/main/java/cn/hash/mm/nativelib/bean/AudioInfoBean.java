package cn.hash.mm.nativelib.bean;

/**
 * Created by Hash on 2020-04-15.
 */


public class AudioInfoBean {

    private int currentTime;

    private int totalTime;


    public int getTotalTime() {
        return totalTime;
    }

    public void setTotalTime(int totalTime) {
        this.totalTime = totalTime;
    }

    public int getCurrentTime() {
        return currentTime;
    }

    public void setCurrentTime(int currentTime) {
        this.currentTime = currentTime;
    }

    @Override
    public String toString() {

        return "audioInfoBean:{" +
                "currentTime:" + currentTime +
                ",totalTime" + totalTime
                + "}";
    }
}

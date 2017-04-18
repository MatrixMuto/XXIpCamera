package com.mut0.xxcam;

/**
 * Created by muto on 17-3-25.
 */

public class XXRtmpPublish {

    public XXRtmpPublish(){

    }


    void addTarget(String url){
        native_add_target(url);
    }


    void connect(){
        native_connect();
    }

    void disconnect(){
        native_disconnect();
    }

    private native void native_add_target(String url);
    private native void native_connect();
    private native void native_disconnect();

}

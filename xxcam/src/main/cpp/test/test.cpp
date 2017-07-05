#include <iostream>


#include "../api/xx_api.h"

using namespace xx;

class cccc : public XXConnectionCallback {

};
void test_play() {
    XXSession *session = XXRtmp::CreateSession();
    /* Handshake? */
    XXConnection *conn = session->CreateConnection("",);

    /* chunk size? window ack size? */
    /* conn == chunk stream? */
//    conn->SetChunkSize(4096);
//    conn->SetWindowAckSize();

    conn->Connect();

    /* create stream*/
    /* app  */
    conn->CreateStream(nullptr);

    XXStream *stream;

    /* publish*/
//    stream->play(name);

    session->Finalize();
}

class XXCustomConnectionCallback : public XXConnectionCallback {
    void OnConnected();

    void OnDisconnectd();
};

class XXSelfStreamCallback : public XXStreamCallback {
public:
    void OnStreamCreated(XXStream *stream) override {

    }

    void OnStreamReleased(XXStream *stream) override {

    }

    void OnPublished(XXStream *stream) override {

    }

    void OnStreamStart(XXStream *stream) override {

    }
};

void test_publish() {
    XXSession *session = XXRtmp::CreateSession();

    /* Handshake? */
    XXConnection *conn = session->CreateConnection("", nullptr);

    /* chunk size? window ack size? */
    /* conn manage chunk stream? */
//    conn->SetChunkSize();
//    conn->WindowsAckSize();
//    conn->SetApplication();

    /* tcp connect, handshake, base type message, acf(connect){app, url, codec } */
    conn->Connect();

    /* create message stream (channel)*/
//    XXStream *stream = ;
    conn->CreateStream(new XXSelfStreamCallback());
    /* publish*/
//    stream->Publish();
    /* meta data */
//    stream->SendVideo(videoFrame)
//    stream->SendAudio(audioFrame);

//    stream->Release();

//    XXStream *stream2 = sesson->CreateStream(conn, stream_cb);

//    stream2->Release();


    conn->Release();

    session->Finalize();
}

int main(int argc, char *argv[]) {
    char c;

    test_publish();

    while (c = getchar()) {

    }
    return 0;
}

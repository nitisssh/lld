/*
    what problem it solves ?
    ** incompatible interfaces 
    ** dealinng with legacy codes 
    ** no sources modification 
*
WHAT MAKES IT WORK ?
** Polymorphism , Composition , Delegation
polymorphism -> since the MediaAdapter inherits the ModernMediaTarget , 
                so it can act as the ModerMedianTarget and can be passed in its place
                since the only thing client knows is ModernMediaAdapter
Compostion -> has - a relationship rather that is - a relationship
            by keeping the legacy player inside the adapter as a tool , the adapter kees its public interface clean 

*/

#include <bits/stdc++.h>
using namespace std ;

/*
    1. The Target ( The interface the Client Expects )
    this is like the US plug in my laptop trying to fit in indian socket
    we only know how to talk to this interface
*/
class ModernMediaTarget {
public:
    virtual ~ModernMediaTarget() = default ;
    virtual void playMp4(string &filename )  = 0 ; 
};

/*
    2. THE ADAPTEE ( The legacy class )
    this class is like indian wall socket , it does the work 
    but it doesn't fit the modern Us plugs

    only knows to play the vlc format but we want to play mp4
*/

class LegacyVlcPlayer {
public:
    void playVlcFormat( string &fileName ) {
        cout << "[LegacyVlcPlayer]: Playing VLC file: " << fileName << endl ; 
    }
};

/*
    3. THE ADAPTER ( the bridge )
    this class inherits from the Target ( so that the client can use it as the ModerMediaTarget  **polymorphism)
    it also contain the instacne of the legacy system inside it so that it can do the actaul work 
*/
class MediaAdapter : public ModernMediaTarget {
private:
    // why void * -> because it acts as a generic pointer
    // void * , a pointer to some block of memory , but it has no idea what is in it
    // it erases the type information completely
    void *genericPlayerPtr ; 
public:
    MediaAdapter() {
        genericPlayerPtr = new LegacyVlcPlayer() ;
    }
    ~MediaAdapter() {
        // we cann't delete a void*
        // sine the copiler doesn't know how big the object is or if it has its own destructor
        // we must cast it back to its true form before deleting it
        LegacyVlcPlayer* actualPlayer = static_cast<LegacyVlcPlayer*>(genericPlayerPtr) ;
        delete actualPlayer ; 
        cout << "MediaAdapter: Safely deleted the generic pointer memory." << std::endl;
    }

    void playMp4 ( string &fileName ) override {
        std::cout << "MediaAdapter: Intercepted mp4 request for '" << fileName << "'." << std::endl;
        std::string translatedFileName = fileName.substr(0, fileName.find(".mp4")) + ".vlc";

        // THE CASTING (The Magic & The Danger)
        // We cannot call 'genericPlayerPtr->playVlcFormat()' because a void* has no methods!
        // We must forcefully tell the compiler: "Trust me, this memory is actually a LegacyVlcPlayer."
        // We use static_cast to convert the generic void* back into a specific typed pointer.
        LegacyVlcPlayer* player = static_cast<LegacyVlcPlayer*>(genericPlayerPtr);

        // Now that the compiler knows what it is again, we can call its functions.
        player->playVlcFormat(translatedFileName);
    }
};

// ==============================================================================
// 4. THE CLIENT (Unchanged)
// ==============================================================================
void clientCode(ModernMediaTarget& target,  std::string file) {
    target.playMp4(file);
}

int main() {
    std::cout << "--- Starting Generic Pointer Application ---" << std::endl;

    MediaAdapter myAdapter ;
    clientCode(myAdapter, "vacation_video.mp4");

    return 0;
}
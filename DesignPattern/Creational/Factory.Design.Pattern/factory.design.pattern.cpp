
#include <bits/stdc++.h>
using namespace std ; 

/*
    NOTIFICATION FACTORY
    ** Product -> Notifier(abstract) : something that can notify ( recipient , message ) 
    ** ConcreteProduct -> EmailNotifier , SMSNotifier , PushNotifier
    ** Creator -> NotificationService ( contains business logic like templating , logging ) 
    ** Factory Method -> makeNotifier( : subclasses decide which notifer to create)
    ** NotificationService::send()

*/
/*
    Product
*/

class Notifier {
    public :
    virtual ~Notifier () {

    }
    virtual void notify ( string recipient , string message ) = 0 ;
};

class EmailNotifier : public Notifier {
    public:
    void notify ( string recipient , string message ) override {
        cout << "[EMAIL] To: " << recipient << " | MSG: " << message << endl ; 
    }
};

class SmsNotifier : public Notifier {
    public:
    void notify ( string recipient , string message ) {
        cout << "[SMS] To: " << recipient << " | MSG: " << message << endl ; 
    }
};

/*
    Factory
*/

class NotificationService {
    public :
    // virtual Notifier makeNotifier() = 0 ; this is an error because Notifier is an abstract class
    // C++ cannot create or move an abstract object , so we cant return it 
    // but we can return a pointer and all
    // this happens because to be able to return Notifier , compiler must need to create the object first 
    // but it is not possible since it is abstract , so we cant return abstract class Notifier
    virtual Notifier * makeNotifier() = 0 ; 

    virtual void specify () = 0 ;
};

class MarketingNotificationService : public NotificationService {
    public :
    Notifier * makeNotifier () override {
        return new EmailNotifier() ;
    }
    void specify () override {
        cout << "[MaketingNotificationService] Preparing to send message..." << endl;
    }
};

class AlertNotificationService : public NotificationService {
public :
    Notifier * makeNotifier () override {
        return new SmsNotifier() ;
    }
    void specify () override {
        cout << "[AlterNotificationService] Preparing to send message..." << endl;
    }
};


int main () {
    NotificationService *serviceMarketing = new MarketingNotificationService() ;
    NotificationService *serviceAlert = new AlertNotificationService() ;

    serviceAlert -> specify() ;
    Notifier *smsNotifierForAlert = serviceAlert -> makeNotifier() ;
    smsNotifierForAlert -> notify ("xyz" , "ghr se na niklna");

    serviceMarketing -> specify() ;
    Notifier *emailNotifierForMarketing = serviceMarketing -> makeNotifier() ;
    emailNotifierForMarketing -> notify ("abs" , "apki aisi taisi") ;
}
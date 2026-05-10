/*
    this design pattern provides the interface to complex system
    allows dealing with complex systems
    helps the client to decouple with the internal complex system
    eg-> deployment engine instead of doinng each step on our own 
        fetching code from rep , running tests ..
    
    WHAT MAKES IT WORK ?
    **Composition , Delegation 
*/

#include <bits/stdc++.h>
using namespace std ; 

// ==========================================
// THE COMPLEX SUBSYSTEMS
// ==========================================

class CodeRepository {
public:
    // Simulates fetching code from GitHub or GitLab.
    // 'std::string' is the return type, representing text.
    std::string fetchLatestCode(std::string branch) {
        std::cout << "Repository: Downloading latest code from branch '" << branch << "'...\n";
        return "source_code_v2";
    }
};

class BuildSystem {
public:
    // Simulates compiling code (like running 'make' or 'npm build').
    // 'bool' returns true (success) or false (failure).
    bool compile(std::string code) {
        std::cout << "BuildSystem: Compiling " << code << " into an executable binary...\n";
        return true; 
    }
};

class TestRunner {
public:
    // Simulates running unit tests.
    bool runTests() {
        std::cout << "TestRunner: Running automated test suite... All tests passed!\n";
        return true;
    }
};

class ServerManager {
public:
    // Simulates uploading the binary and restarting the live server.
    void deployToProduction() {
        std::cout << "ServerManager: Uploading binary. Restarting production servers...\n";
    }
};

// THE FACADe
class DeploymentFacade {
private:
    // keeping themm private so that client code doesn't mess with dem 
    CodeRepository* repo ; 
    BuildSystem* builder ; 
    TestRunner* tester ; 
    ServerManager* server ; 
public:
    DeploymentFacade(){
        repo = new CodeRepository() ;
        builder = new BuildSystem() ;
        tester = new TestRunner() ;
        server = new ServerManager() ;
    }
    ~DeploymentFacade() {
        // SYNTAX: The 'delete' keyword frees the memory that was previously allocated with 'new'.
        delete repo;
        delete builder;
        delete tester;
        delete server;
        std::cout << "(System: Cleaned up deployment subsystems from memory.)\n";
    }

    // the facade method , exposed to the client
    void deployApp( string branchname ) {
        std::cout << "\n=== FACADE: Starting Deployment Pipeline ===\n";
        std::string code = repo->fetchLatestCode(branchname);

        if (!builder->compile(code)) {
            std::cout << "FACADE ERROR: Build failed. Aborting deployment.\n";
            return; 
        }
        
        if (!tester->runTests()) {
            std::cout << "FACADE ERROR: Tests failed. Aborting deployment.\n";
            return;
        }
        
        server->deployToProduction();    
        std::cout << "=== FACADE: Deployment Pipeline Complete ===\n\n";
    }
};

int main() {
    DeploymentFacade onClickDeploy ; 
    onClickDeploy.deployApp("main") ;
}
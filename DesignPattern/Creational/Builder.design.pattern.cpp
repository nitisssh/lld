/*
  what problem does it solve ??
  its like ordeiring a subaway with custom addons
  if we didn't use this structure we would have to use multiple overloaded 
  constructor for each combination of addons 
  that would be hell 

  here with builder design patter we can create a complex objects step by step
  seperating the construction logic with the final representattion
  ** deals with many optional feilds
  ** telescoping constructors

  eg -> SqlQueryBuilders that ORMS use
        HttpUrlBuilder
        Desktop builder 
WHAT MAKES BUILDER DESIGN PATTERN WORK ?
*** Encapsulation and Method Chaining ( return *this )
Encapsulation -> because , the builder holds the The product we are trying to build
                and since the properties are private , doesn't let any other clien build the product directly without builder
return *this -> allows chaining
*/

#include <bits/stdc++.h>
using namespace std ; 


/* using class here helps us to not allow use any enums as variable elsewhere */
enum class HttpMethod {
  GET , POST , PUT , DELETE 
};

string methodToString ( HttpMethod method ) { 
  switch ( method ) {
    case HttpMethod::GET: return "GET" ;
    case HttpMethod::PUT: return "PUT" ;
    case HttpMethod::DELETE: return "DELETE" ;
    case HttpMethod::POST: return "POST" ;
  }
};

/* the producct HttpRequest */

class HttpRequest {
private :
  HttpMethod method ; 
  string url ;
  map < string , string > headers , queryParams ; 
  string body ; 
  int timeOuts ; 
  /* why is the constructor private ?
    * we want the user to use the HttpBuilder Class 
    * so that we can avoid that the user creating half baked urls that doesnt make any sense
  */
  HttpRequest() : method (HttpMethod::GET) , timeOuts(3000) {}

  friend class HttpRequestBuilder ;

public :
  void debugPrint() const {
          std::cout << "--- HTTP Request ---\n";
          std::cout << methodToString(method) << " " << url << " HTTP/1.1\n";
          for (const auto& header : headers) {
              std::cout << header.first << ": " << header.second << "\n";
          }
          std::cout << "Timeout: " << timeOuts << "ms\n";
          if (!body.empty()) {
              std::cout << "\n[Body]\n" << body << "\n";
          }
          std::cout << "--------------------\n\n";
      }
};

class HttpRequestBuilder {
private :
  HttpRequest request ; 
public:
  /*
    why we return the "HttpReqeustBuilder" and " return *this" ?
    * this lets us to return the referece to the current builder itslef 
    * this is the pointer to the current builder object and *this dereferces it to return the actual object
    * this allows us to add property on top of other and allows chaining
    * like builder.setMethod().setUrl().setBody( )..
    * if we returned the void we would have to use the builder again and agina
    * like HttpRequestBuilder builder ;
    * builder.setMethod();
    * builder.setUrl() ;
    *  .. we couldn't chain directly
  */
  HttpRequestBuilder& setMethod ( HttpMethod method ) {
    request.method = method ;
    return *this ;
  }
  HttpRequestBuilder& setUrl ( string url ) {
    request.url = url ; 
    return *this ;
  }
  HttpRequestBuilder& setBody ( string body ) {
    request.body = body ; 
    return *this ;
  }
  HttpRequestBuilder& setHeader ( string key , string value ) {
    request.headers[key] = value ; 
    return *this ;
  }
  HttpRequestBuilder& setTimeout(int milliseconds) {
      request.timeOuts = milliseconds;
      return *this;
  }
  HttpRequestBuilder& setQueryParams ( string key , string value ) {
    request.queryParams[key] = value ;
    return *this ;
  }
  /* why are we returning by value here ?
     * once the build is complete , we want the detach the product fromt he builder 
     * by returning the copy of the final request object 
     * the user gets the independent object , can modify it , delete it etc
     * other wise they could modify the curent object they just built
  */
  HttpRequest build() {
    if (request.url.empty()) {
        throw std::runtime_error("Cannot build HttpRequest: URL is missing!");
    }
    bool first = false ; 
    for ( auto &query : request.queryParams ) {
      if ( first == false ) {
        first = true ; 
        request.url += "?" ;
      }
      request.url += query.first ; 
      request.url += "=" ,
      request.url += query.second ;
      request.url += "&" ;
    }
    if ( first ) {
      request.url.pop_back() ;
    }
    return request;
  }
}
;


/* The client code */

int main () {
  HttpRequestBuilder Builder ; 
  HttpRequest Req = Builder.setUrl("https://api.example.com/users")
                            .setHeader("Accept" , "application/json")
                            .setQueryParams("id" , "434")
                            .setQueryParams("page" , "3")
                            .build() ;
  cout << "Sending GET request...\n";
  Req.debugPrint();
  HttpRequestBuilder postBuilder;
  HttpRequest postReq = postBuilder.setMethod(HttpMethod::POST)
                                    .setUrl("https://api.example.com/login")
                                    .setHeader("Content-Type", "application/json")
                                    .setBody("{\"username\":\"admin\"}")
                                    .build();

  std::cout << "Sending POST request...\n";
  postReq.debugPrint();

  std::cout << "Trying to build an invalid request...\n";
  HttpRequestBuilder badBuilder;
  HttpRequest badReq = badBuilder.setMethod(HttpMethod::GET).build();
}

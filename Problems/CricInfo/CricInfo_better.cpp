/*
Requirements
The Cricinfo system should provide information about cricket matches, teams, players, and live scores.
Users should be able to view the schedule of upcoming matches and the results of completed matches.
The system should allow users to search for specific matches, teams, or players.
Users should be able to view detailed information about a particular match, including the scorecard, commentary, and statistics.
The system should support real-time updates of live scores and match information.
The system should handle concurrent access to match data and ensure data consistency.
The system should be scalable and able to handle a large volume of user requests.
The system should be extensible to accommodate new features and enhancements in the future.
*/ 

#include <bits/stdc++.h>
#include <random>
#include <iomanip>
#include "../../SearchEngine/search_engine_api.h"
using namespace std;

class Player {
    private:
    int id ; 
    int team_id ; 
    string name ;
    public:
    Player(){}
    Player(int id , string name , int teamId ) :id(id) , team_id(teamId) , name(name) {}
    int getId () {
        return id ;
    }
    string getName() {
        return name ;
    }
    int getTeamId() {
        return team_id ;
    }
};

// need to create a player factory do i ?
// no ideally 

struct PlayerStats {
    int runs = 0;
    int wickets = 0;
    int dot_balls = 0;
    int sixes = 0;
    int fours = 0;
    int runouts = 0;
    int catches = 0;
    int matches_played = 0;
};

struct PlayerMatchStats {
    string playerName;
    int runs;
    int wickets;
    int catches;
    int runouts;
};

struct TeamScore {
    string teamName;
    int runs;
    int wickets;
};

struct Scorecard {
    int matchId;
    TeamScore team1Score;
    TeamScore team2Score;
    vector<PlayerMatchStats> playerStats;
};

// strategy design pattern
class IPlayerRepository {
public:
    virtual ~IPlayerRepository() = default;
    virtual void addEvent(int player_id, int runs, int wicket, int isCatch, 
                          int isRunout, int team_id, int match_id) = 0;
    virtual void addPlayer(Player *p) = 0;
    virtual Player *fetchPlayer(int id) = 0;
    virtual vector<int> getTeamPlayers(int team_id) = 0;
    virtual PlayerStats getPlayerTotalStats(int player_id) = 0;
    virtual vector<Player*> getAllPlayers() = 0;
};

class InMemorySqlRepository : public IPlayerRepository {
private:
    unordered_map<int, PlayerStats> total_stats;
    unordered_map<int, unordered_map<int, PlayerStats>> match_stats;
    unordered_map<int, Player*> playerMappinng ;
    unordered_map<int, vector<int>> teamPlayers;
public:
    InMemorySqlRepository() = default;

    void addEvent(int player_id, int runs, int wicket, int isCatch, 
                  int isRunout, int team_id, int match_id) override {
        
        total_stats[player_id].runs += runs;
        total_stats[player_id].wickets += wicket;
        total_stats[player_id].catches += isCatch;
        total_stats[player_id].runouts += isRunout;
        
        if (runs == 0) total_stats[player_id].dot_balls++;
        if (runs == 4) total_stats[player_id].fours++;
        if (runs == 6) total_stats[player_id].sixes++;

        auto& current_match = match_stats[player_id][match_id];
        current_match.runs += runs;
        current_match.wickets += wicket;
        current_match.catches += isCatch;
        current_match.runouts += isRunout;
        
        if (runs == 0) current_match.dot_balls++;
        if (runs == 4) current_match.fours++;
        if (runs == 6) current_match.sixes++;
        
        cout << "[Repo Log] Event added successfully for Player: " << player_id << "\n";
    }

    Player *fetchPlayer (int id) override {
        if (playerMappinng.count(id) == 0) {
            cout <<"[Repo Log] Invalid Request .. Player Doesn't Exist!" << endl ;
        }
        return playerMappinng[id] ;
    }
    void addPlayer (Player *p) override {
        playerMappinng[p->getId()] = p ;
        teamPlayers[p->getTeamId()].push_back(p->getId());
    }
    vector<int> getTeamPlayers(int team_id) override {
        return teamPlayers[team_id];
    }
    PlayerStats getPlayerTotalStats(int player_id) override {
        return total_stats[player_id];
    }
    vector<Player*> getAllPlayers() override {
        vector<Player*> all;
        for (auto& pair : playerMappinng) {
            all.push_back(pair.second);
        }
        return all;
    }
};

class PlayerService {
    private:
    IPlayerRepository *repo;
    public:
    PlayerService(IPlayerRepository *repo) : repo(repo) {}
    Player* create(int id , string name , int teamId) {
        Player *p = new Player(id , name , teamId) ;
        repo->addPlayer(p) ;
        return p;
    }
};

class Team {
    private:
    int id ;
    string name;
    vector < int > players ; // stores the player ids 
    public:
    Team(int id , string name) {
        this -> id = id ;
        this -> name = name ;
    }
    Team() {}
    void addPlayer (int id) {
        cout << "[Team Log] Player " << id << " added to team " << this->id << endl ;
        players.push_back(id) ;
    }
    vector < int > showPlayers() {
        return players ;
    }
    int getId () {
        return id ;
    }
    string getName() {
        return name ;
    }
};

struct MatchTeam {
    int id1 , id2 ;
};

enum class MatchType {
    ODIS , T20s , Test 
};

// since the match rules and behaviour changes according to the type , we should use inheritance here
class IMatch {
    protected:
    int id ;
    public:
    MatchTeam t ;
    protected:
    int totalInnings ;
    int overs ;
    public:
    IMatch(int id, MatchTeam t, int totalInnings) : id(id), t(t), totalInnings(totalInnings) {}
    virtual ~IMatch () = default ; 
    virtual void start() = 0 ;
    int getId() const { return id; }
    int getTotalInnings() const { return totalInnings; }
    int getOvers() const { return overs; }
};

class T20 : public IMatch {
    public :
    T20( int id , int totInnings , MatchTeam t ) : IMatch(id, t, totInnings) {
        overs = 20 ;
    }
    void start () override {
        cout << "[MATCH] T20 Match Started between " << t.id1 << " and " <<  t.id2 << endl ;
    }
};

class Test : public IMatch {
    public :
    Test( int id , int totInnings , MatchTeam t ) : IMatch(id, t, totInnings) {
        overs = 90;
    }
    void start () override {
        cout << "[MATCH] Test Match Started between " << t.id1 << " and " <<  t.id2 << endl ;
    }
};

// factory to create matches of different types 
class IMatchFactory {
    public:
    virtual ~IMatchFactory() = default ;
    virtual IMatch * create(int id , int totInnings , MatchTeam t) = 0 ; // pointer is necessary since IMatch is an abstract class and it doesn't have implementation 
};

class T20Factory : public IMatchFactory {
    public:
    IMatch * create(int id , int totInnings , MatchTeam t) override {
        cout << "T20 Match Created " << id << " TotInnings: " << totInnings << " T1: " << t.id1 << " T2: " << t.id2 << endl ;
        return new T20(id , totInnings , t) ;
    }
};

class TestFactory : public IMatchFactory {
    public:
    IMatch * create(int id , int totInnings , MatchTeam t) override {
        cout << "Test Match Created " << id << " TotInnings: " << totInnings << " T1: " << t.id1 << " T2: " << t.id2 << endl ;
        return new Test(id , totInnings , t) ;
    }
};

class TeamRepository {
private:
    unordered_map<int, Team*> teams;
public:
    void saveTeam(Team* t) {
        teams[t->getId()] = t;
    }
    Team* getTeam(int id) {
        if (teams.count(id) == 0) return nullptr;
        return teams[id];
    }
    vector<Team*> getAllTeams() {
        vector<Team*> all;
        for (auto& pair : teams) {
            all.push_back(pair.second);
        }
        return all;
    }
};

class MatchRepository {
private:
    unordered_map<int, Scorecard> scorecards;
    unordered_map<int, IMatch*> matches;
public:
    void saveMatch(IMatch* m) {
        matches[m->getId()] = m;
    }
    IMatch* getMatch(int matchId) {
        if (matches.count(matchId) == 0) return nullptr;
        return matches[matchId];
    }
    void saveScorecard(int matchId, const Scorecard& sc) {
        scorecards[matchId] = sc;
    }
    bool hasScorecard(int matchId) const {
        return scorecards.count(matchId) > 0;
    }
    Scorecard getScorecard(int matchId) {
        return scorecards[matchId];
    }
};

class MatchService {
    private:
    IMatch *match;
    IPlayerRepository *playerRepo ;
    MatchRepository *matchRepo ;
    public:
    MatchService(IPlayerRepository *repo , IMatch *m, MatchRepository* mRepo = nullptr) 
        : playerRepo(repo) , match(m), matchRepo(mRepo) {}
    // toss 
    void simulate() {
        std::random_device rd ;// entropy source 
        std::mt19937 gen(rd()) ;// gerenate a random no in range like very large
        std::uniform_int_distribution<int> d(0 , 1) ;
        int winner = d(gen) ;
        int isBatting = d(gen) ;
        
        int team1 = match->t.id1;
        int team2 = match->t.id2;
        
        int tossWinner = winner ? team1 : team2;
        int firstBattingTeam, secondBattingTeam;
        
        if (winner) {
            cout << "[TOSS WON BY : ] " << team1 << endl ;
            if (isBatting) {
                cout << "[TOSS CHOICE] Team " << team1 << " chose to bat first." << endl;
                firstBattingTeam = team1;
                secondBattingTeam = team2;
            } else {
                cout << "[TOSS CHOICE] Team " << team1 << " chose to bowl first." << endl;
                firstBattingTeam = team2;
                secondBattingTeam = team1;
            }
        } else {
            cout << "[TOSS WON BY : ] " << team2 << endl ;
            if (isBatting) {
                cout << "[TOSS CHOICE] Team " << team2 << " chose to bat first." << endl;
                firstBattingTeam = team2;
                secondBattingTeam = team1;
            } else {
                cout << "[TOSS CHOICE] Team " << team2 << " chose to bowl first." << endl;
                firstBattingTeam = team1;
                secondBattingTeam = team2;
            }
        }
        
        cout << "\n[SIMULATION START] Beginning simulation of match " << match->getId() << endl;
        match->start();
        
        int inningsToSimulate = match->getTotalInnings();
        int firstInningsRuns = 0;
        
        for (int innings = 1; innings <= inningsToSimulate; ++innings) {
            int batTeam = (innings % 2 == 1) ? firstBattingTeam : secondBattingTeam;
            int bowlTeam = (innings % 2 == 1) ? secondBattingTeam : firstBattingTeam;
            
            vector<int> batPlayers = playerRepo->getTeamPlayers(batTeam);
            vector<int> bowlPlayers = playerRepo->getTeamPlayers(bowlTeam);
            
            if (batPlayers.size() < 2 || bowlPlayers.empty()) {
                cout << "[ERROR] Insufficient players for simulation!" << endl;
                return;
            }
            
            cout << "\n--- Innings " << innings << ": Team " << batTeam << " Batting ---" << endl;
            
            int totalRuns = 0;
            int totalWickets = 0;
            int strikerIdx = 0;
            int nonStrikerIdx = 1;
            int nextBatsmanIdx = 2;
            
            int totalBalls = match->getOvers() * 6;
            std::uniform_int_distribution<int> playDist(0, 100);
            
            for (int ball = 1; ball <= totalBalls; ++ball) {
                if (totalWickets >= 10) break;
                
                if (innings == 2 && inningsToSimulate == 2 && totalRuns > firstInningsRuns) {
                    break;
                }
                
                int overNum = (ball - 1) / 6;
                int bowlerIdx = 6 + (overNum % 5);
                if (bowlerIdx >= bowlPlayers.size()) bowlerIdx = bowlPlayers.size() - 1;
                int bowlerId = bowlPlayers[bowlerIdx];
                int strikerId = batPlayers[strikerIdx];
                
                int val = playDist(gen);
                
                if (val < 35) {
                    playerRepo->addEvent(strikerId, 0, 0, 0, 0, batTeam, match->getId());
                } else if (val < 65) {
                    playerRepo->addEvent(strikerId, 1, 0, 0, 0, batTeam, match->getId());
                    totalRuns += 1;
                    swap(strikerIdx, nonStrikerIdx);
                } else if (val < 75) {
                    playerRepo->addEvent(strikerId, 2, 0, 0, 0, batTeam, match->getId());
                    totalRuns += 2;
                } else if (val < 78) {
                    playerRepo->addEvent(strikerId, 3, 0, 0, 0, batTeam, match->getId());
                    totalRuns += 3;
                    swap(strikerIdx, nonStrikerIdx);
                } else if (val < 88) {
                    playerRepo->addEvent(strikerId, 4, 0, 0, 0, batTeam, match->getId());
                    totalRuns += 4;
                } else if (val < 93) {
                    playerRepo->addEvent(strikerId, 6, 0, 0, 0, batTeam, match->getId());
                    totalRuns += 6;
                } else {
                    totalWickets++;
                    std::uniform_int_distribution<int> wicketDist(0, 9);
                    int wType = wicketDist(gen);
                    
                    if (wType < 6) {
                        std::uniform_int_distribution<int> fielderDist(0, bowlPlayers.size() - 1);
                        int fielderId = bowlPlayers[fielderDist(gen)];
                        playerRepo->addEvent(strikerId, 0, 0, 0, 0, batTeam, match->getId());
                        playerRepo->addEvent(bowlerId, 0, 1, 0, 0, bowlTeam, match->getId());
                        playerRepo->addEvent(fielderId, 0, 0, 1, 0, bowlTeam, match->getId());
                        cout << "[OUT] " << playerRepo->fetchPlayer(strikerId)->getName() 
                             << " caught by " << playerRepo->fetchPlayer(fielderId)->getName() 
                             << " off " << playerRepo->fetchPlayer(bowlerId)->getName() << endl;
                    } else if (wType < 7) {
                        std::uniform_int_distribution<int> fielderDist(0, bowlPlayers.size() - 1);
                        int fielderId = bowlPlayers[fielderDist(gen)];
                        int outIdx = (wicketDist(gen) % 2 == 0) ? strikerIdx : nonStrikerIdx;
                        int outId = batPlayers[outIdx];
                        playerRepo->addEvent(outId, 0, 0, 0, 0, batTeam, match->getId());
                        playerRepo->addEvent(fielderId, 0, 0, 0, 1, bowlTeam, match->getId());
                        cout << "[OUT] " << playerRepo->fetchPlayer(outId)->getName() 
                             << " run out by " << playerRepo->fetchPlayer(fielderId)->getName() << endl;
                        
                        if (outIdx == strikerIdx) {
                            strikerIdx = -1;
                        } else {
                            nonStrikerIdx = -1;
                        }
                    } else {
                        playerRepo->addEvent(strikerId, 0, 0, 0, 0, batTeam, match->getId());
                        playerRepo->addEvent(bowlerId, 0, 1, 0, 0, bowlTeam, match->getId());
                        cout << "[OUT] " << playerRepo->fetchPlayer(strikerId)->getName() 
                             << " bowled by " << playerRepo->fetchPlayer(bowlerId)->getName() << endl;
                    }
                    
                    if (totalWickets < 10) {
                        if (strikerIdx == -1) {
                            strikerIdx = nextBatsmanIdx++;
                        } else {
                            nonStrikerIdx = nextBatsmanIdx++;
                        }
                    }
                }
                
                if (ball % 6 == 0 && ball < totalBalls && totalWickets < 10) {
                    swap(strikerIdx, nonStrikerIdx);
                }
            }
            
            cout << "Team " << batTeam << " Innings Summary: " << totalRuns << "/" << totalWickets << endl;
            if (innings == 1) {
                firstInningsRuns = totalRuns;
            } else {
                if (totalRuns > firstInningsRuns) {
                    cout << "Team " << batTeam << " won the match!" << endl;
                } else if (totalRuns < firstInningsRuns) {
                    cout << "Team " << firstBattingTeam << " won the match!" << endl;
                } else {
                    cout << "Match Tied!" << endl;
                }
            }
        }
        
        cout << "\n================ SCORECARD ================" << endl;
        for (int teamId : {firstBattingTeam, secondBattingTeam}) {
            cout << "\nTeam " << teamId << " Player Stats:" << endl;
            cout << "--------------------------------------------------------" << endl;
            cout << left << setw(20) << "Name" << setw(10) << "Runs" << setw(10) << "Wickets" << setw(10) << "Catches" << setw(10) << "Runouts" << endl;
            cout << "--------------------------------------------------------" << endl;
            vector<int> teamPlayers = playerRepo->getTeamPlayers(teamId);
            for (int pId : teamPlayers) {
                Player* p = playerRepo->fetchPlayer(pId);
                PlayerStats stats = playerRepo->getPlayerTotalStats(pId);
                cout << left << setw(20) << p->getName() 
                     << setw(10) << stats.runs 
                     << setw(10) << stats.wickets 
                     << setw(10) << stats.catches 
                     << setw(10) << stats.runouts << endl;
            }
            cout << "--------------------------------------------------------" << endl;
        }
        cout << "===========================================" << endl;

        // Save Scorecard to MatchRepository
        if (matchRepo) {
            Scorecard sc;
            sc.matchId = match->getId();
            
            sc.team1Score.teamName = "Team " + to_string(team1);
            sc.team1Score.runs = 0;
            sc.team1Score.wickets = 0;
            vector<int> t1Players = playerRepo->getTeamPlayers(team1);
            for (int pId : t1Players) {
                Player* p = playerRepo->fetchPlayer(pId);
                PlayerStats stats = playerRepo->getPlayerTotalStats(pId);
                PlayerMatchStats pms = {p->getName(), stats.runs, stats.wickets, stats.catches, stats.runouts};
                sc.playerStats.push_back(pms);
                sc.team1Score.runs += stats.runs;
                sc.team1Score.wickets += stats.wickets;
            }
            
            sc.team2Score.teamName = "Team " + to_string(team2);
            sc.team2Score.runs = 0;
            sc.team2Score.wickets = 0;
            vector<int> t2Players = playerRepo->getTeamPlayers(team2);
            for (int pId : t2Players) {
                Player* p = playerRepo->fetchPlayer(pId);
                PlayerStats stats = playerRepo->getPlayerTotalStats(pId);
                PlayerMatchStats pms = {p->getName(), stats.runs, stats.wickets, stats.catches, stats.runouts};
                sc.playerStats.push_back(pms);
                sc.team2Score.runs += stats.runs;
                sc.team2Score.wickets += stats.wickets;
            }
            matchRepo->saveScorecard(match->getId(), sc);
        }
    }
};

class SearchService {
    private:
    IPlayerRepository *playerRepo ;
    TeamRepository *teamRepo;
    MatchRepository *matchRepo;
    public:
    SearchService(IPlayerRepository *repo, TeamRepository* teamRepo, MatchRepository* matchRepo) 
        : playerRepo(repo), teamRepo(teamRepo), matchRepo(matchRepo) {}

    vector<Player*> searchPlayer(const string& query) {
        vector<Player*> results;
        for (Player* p : playerRepo->getAllPlayers()) {
            if (p->getName().find(query) != string::npos) {
                results.push_back(p);
            }
        }
        return results;
    }

    vector<Team*> searchTeam(const string& query) {
        vector<Team*> results;
        for (Team* t : teamRepo->getAllTeams()) {
            if (t->getName().find(query) != string::npos) {
                results.push_back(t);
            }
        }
        return results;
    }

    vector<string> searchSemantic(const vector<float>& query_vector, int k) {
        return HNSWEngine::Search(query_vector, k, 64);
    }
};

class CricInfoFacade;

class User {
    private:
    int id ;
    string name;
    public:
    User(int id, string name = "Guest") : id(id), name(name) {}
    string getName() const { return name; }
    int getId() const { return id; }
    
    void searchPlayer(CricInfoFacade* facade, const string& playerName);
    void searchTeam(CricInfoFacade* facade, const string& teamName);
    void viewScorecard(CricInfoFacade* facade, int matchId);
};

class CricInfoFacade {
private:
    IPlayerRepository* playerRepo;
    TeamRepository* teamRepo;
    MatchRepository* matchRepo;
    PlayerService* playerService;
    SearchService* searchService;

public:
    CricInfoFacade() {
        playerRepo = new InMemorySqlRepository();
        teamRepo = new TeamRepository();
        matchRepo = new MatchRepository();
        playerService = new PlayerService(playerRepo);
        searchService = new SearchService(playerRepo, teamRepo, matchRepo);
    }
    
    CricInfoFacade(IPlayerRepository* pRepo, TeamRepository* tRepo, MatchRepository* mRepo, PlayerService* pServ) 
        : playerRepo(pRepo), teamRepo(tRepo), matchRepo(mRepo), playerService(pServ) {
        searchService = new SearchService(playerRepo, teamRepo, matchRepo);
    }

    ~CricInfoFacade() {
        delete searchService;
    }

    Player* createPlayer(int id, string name, int teamId) {
        return playerService->create(id, name, teamId);
    }

    void registerTeam(Team* t) {
        teamRepo->saveTeam(t);
    }

    void registerMatch(IMatch* m) {
        matchRepo->saveMatch(m);
    }

    void simulateMatch(IMatch* m) {
        MatchService* simulator = new MatchService(playerRepo, m, matchRepo);
        simulator->simulate();
        delete simulator;
    }

    void displayScorecard(int matchId) {
        if (!matchRepo->hasScorecard(matchId)) {
            cout << "No scorecard found for Match ID: " << matchId << endl;
            return;
        }
        Scorecard sc = matchRepo->getScorecard(matchId);
        cout << "\n=========== FACADE SCORECARD (Match " << sc.matchId << ") ===========" << endl;
        cout << sc.team1Score.teamName << ": " << sc.team1Score.runs << " runs" << endl;
        cout << sc.team2Score.teamName << ": " << sc.team2Score.runs << " runs" << endl;
        cout << "--------------------------------------------------------" << endl;
        cout << left << setw(20) << "Name" << setw(10) << "Runs" << setw(10) << "Wickets" << setw(10) << "Catches" << setw(10) << "Runouts" << endl;
        cout << "--------------------------------------------------------" << endl;
        for (const auto& pms : sc.playerStats) {
            cout << left << setw(20) << pms.playerName 
                 << setw(10) << pms.runs 
                 << setw(10) << pms.wickets 
                 << setw(10) << pms.catches 
                 << setw(10) << pms.runouts << endl;
        }
        cout << "========================================================" << endl;
    }

    void searchPlayer(const string& name) {
        vector<Player*> players = searchService->searchPlayer(name);
        if (players.empty()) {
            cout << "No players found matching \"" << name << "\"" << endl;
        } else {
            for (Player* p : players) {
                cout << "Found Player: " << p->getName() << " (ID: " << p->getId() << ", Team ID: " << p->getTeamId() << ")" << endl;
            }
        }
    }

    void searchTeam(const string& name) {
        vector<Team*> teams = searchService->searchTeam(name);
        if (teams.empty()) {
            cout << "No teams found matching \"" << name << "\"" << endl;
        } else {
            for (Team* t : teams) {
                cout << "Found Team: " << t->getName() << " (ID: " << t->getId() << ")" << endl;
            }
        }
    }
    
    void searchSemantic(const vector<float>& query_vector, int k = 5) {
        vector<string> results = searchService->searchSemantic(query_vector, k);
        for (int i = 0; i < results.size(); ++i) {
            cout << i + 1 << ". " << results[i] << endl;
        }
    }
};

inline void User::searchPlayer(CricInfoFacade* facade, const string& playerName) {
    cout << "\n[User " << name << " (ID: " << id << ") Search Player Request]: \"" << playerName << "\"" << endl;
    facade->searchPlayer(playerName);
}

inline void User::searchTeam(CricInfoFacade* facade, const string& teamName) {
    cout << "\n[User " << name << " (ID: " << id << ") Search Team Request]: \"" << teamName << "\"" << endl;
    facade->searchTeam(teamName);
}

inline void User::viewScorecard(CricInfoFacade* facade, int matchId) {
    cout << "\n[User " << name << " (ID: " << id << ") View Scorecard Request]: Match " << matchId << endl;
    facade->displayScorecard(matchId);
}
int main() {
    const int dimension = 768 ;
    const string embedding_bin_path = "embeddings_sports.bin";
    const string mapping_txt = "mapping_sports.txt" ;
    const string output_db = "../../SearchEngine/embeddings_store.db" ;
    if (!HNSWEngine::InitializeFromLegacy(embedding_bin_path , mapping_txt , output_db , dimension)) {
        cerr << "Initialisation failed " << endl ; 
        return 1 ;
    }
    cout << "Engine Initialisation Scuccessfull intitialised from legacy files" << endl ;
    cout << "Active Node Count in HNSW : " <<  HNSWEngine::GetNodeCount() << endl ;

    // strategy to create sql repository for player 
    InMemorySqlRepository* playerRepo = new InMemorySqlRepository() ;
    // Player service to handle creation and other stats
    PlayerService* pService = new PlayerService(playerRepo);

    // Create Team and Match Repositories
    TeamRepository* teamRepo = new TeamRepository();
    MatchRepository* matchRepo = new MatchRepository();

    // Create Facade
    CricInfoFacade* cricinfo = new CricInfoFacade(playerRepo, teamRepo, matchRepo, pService);

    int playerID = 1 , teamId = 1 , matchId = 1 ; 

    // player names 
    vector<string> india_players = {
    "Rohit Sharma",
    "Virat Kohli",
    "Jasprit Bumrah",
    "Shubman Gill",
    "Shreyas Iyer",
    "KL Rahul",
    "Hardik Pandya",
    "Ravindra Jadeja",
    "Mohammed Siraj",
    "Kuldeep Yadav",
    "Suryakumar Yadav"
    };

    // Australian Cricket Team Players
    vector<string> australia_players = {
    "Pat Cummins",
    "Travis Head",
    "Mitchell Marsh",
    "Steve Smith",
    "Marnus Labuschagne",
    "Glenn Maxwell",
    "Marcus Stoinis",
    "Alex Carey",
    "Mitchell Starc",
    "Josh Hazlewood",
    "Adam Zampa"
    };

    //Create Teams
    Team *teamInd = new Team ( teamId ++ , " INDIA ");
    Team *teamAus = new Team ( teamId ++  , " AUSTRALIA ");

    cricinfo->registerTeam(teamInd);
    cricinfo->registerTeam(teamAus);

    for ( int i = 0 ; i < 11 ; i ++ ) {
        cricinfo->createPlayer(i , india_players[i] , teamInd->getId() );
        teamInd->addPlayer(playerID ++) ;
    }

    for ( int i = 0 ; i < 11 ; i ++ ) {
        cricinfo->createPlayer(i + 11 , australia_players[i] , teamAus->getId() );
        teamAus->addPlayer(playerID ++) ;
    }

    // create a team for match
    MatchTeam mt = {teamInd -> getId() , teamAus -> getId()} ;
    
    //create match creating factory 
    IMatchFactory *t20factory = new T20Factory();
    IMatchFactory *testfactory = new TestFactory();

    // create a match using factory
    IMatch *t20match = t20factory -> create(matchId ++ , 1 , mt);
    IMatch *testmatch = testfactory -> create(matchId ++ , 2 , mt);

    cricinfo->registerMatch(t20match);
    cricinfo->registerMatch(testmatch);

    cout << "\n===========================================" << endl;
    cout << "SIMULATING T20 MATCH" << endl;
    cout << "===========================================" << endl;
    cricinfo->simulateMatch(t20match);

    cout << "\n===========================================" << endl;
    cout << "SIMULATING TEST MATCH" << endl;
    cout << "===========================================" << endl;
    cricinfo->simulateMatch(testmatch);

    // Create a User
    User user(101, "Sachin");

    // Search for a player
    user.searchPlayer(cricinfo, "Virat");
    
    // Search for a team
    user.searchTeam(cricinfo, "INDIA");

    // View scorecard of the T20 match
    user.viewScorecard(cricinfo, t20match->getId());

    // Clean up
    delete cricinfo;
    delete teamRepo;
    delete matchRepo;
    delete pService;
    delete playerRepo;
    delete t20match;
    delete testmatch;
    delete t20factory;
    delete testfactory;
}
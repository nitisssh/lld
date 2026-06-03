#include <iostream>
#include <vector>
#include <unordered_map>
#include <string>
#include <optional>
#include <algorithm>
#include <random>
#include <stdexcept>
#include "../SearchEngine/search_engine_api.h"

using namespace std;

// ============================================================================
// 1. CORE DOMAIN TYPES & ENUMS
// ============================================================================

enum class Genre {
    METAL, POP, SAD, PARTY, HIPHOP
};

enum class PlaybackEvent { 
    START, PAUSE, STOP, SKIP 
};

class Song {
private:
    int id;
    int artist_id;
    Genre genre;
    string name;
    string master_manifest_key;
    bool active;

public:
    Song() : id(0), artist_id(0), genre(Genre::METAL), name(""), master_manifest_key(""), active(false) {}
    Song(int id, int artist_id, Genre genre, const string& name, const string& key, bool is_active)
        : id(id), artist_id(artist_id), genre(genre), name(name), master_manifest_key(key), active(is_active) {}

    int getId() const { return id; }
    int getArtistId() const { return artist_id; }
    Genre getGenre() const { return genre; }
    bool isActive() const { return active; }
    string getName() const { return name; }
    string getMasterManifestKey() const { return master_manifest_key; }
};

class Playlist {
private:
    int id;
    int user_id;
    vector<int> song_ids;
    bool shuffle_enabled;

public:
    Playlist() : id(0), user_id(0), shuffle_enabled(false) {}
    Playlist(int id, int user_id) : id(id), user_id(user_id), shuffle_enabled(false) {}

    int getId() const { return id; }
    int getUserId() const { return user_id; }
    const vector<int>& getSongIds() const { return song_ids; }
    bool shouldShuffle() const { return shuffle_enabled; }
    void setShuffle(bool val) { shuffle_enabled = val; }

    bool songExists(int song_id) const {
        return find(song_ids.begin(), song_ids.end(), song_id) != song_ids.end();
    }

    void addSong(int song_id) {
        if (songExists(song_id)) return;
        song_ids.push_back(song_id);
    }

    void deleteSong(int song_id) {
        song_ids.erase(remove(song_ids.begin(), song_ids.end(), song_id), song_ids.end());
    }
};

// ============================================================================
// 2. FACTORY PATTERN (MEDIA INGESTION)
// ============================================================================

class MediaFactory {
public:
    virtual ~MediaFactory() = default;
    virtual Song* createTrack(int id, int artist_id, Genre genre, const string& name, const string& key) const = 0;
};

class StandardMediaFactory : public MediaFactory {
public:
    Song* createTrack(int id, int artist_id, Genre genre, const string& name, const string& key) const override {
        if (name.empty() || key.empty()) {
            throw invalid_argument("[FACTORY ERROR] Track name and manifest key cannot be empty.");
        }
        // Returns heap-allocated raw pointer. Caller (Repository) assumes ownership.
        return new Song(id, artist_id, genre, name, key, true);
    }
};

// ============================================================================
// 3. OBSERVER PATTERN (DECOUPLED TELEMETRY & ANALYTICS)
// ============================================================================

class IPlaybackObserver {
public:
    virtual ~IPlaybackObserver() = default;
    virtual void onPlaybackEvent(PlaybackEvent event, int user_id, int song_id) = 0;
};

class AudioAnalyticsService : public IPlaybackObserver {
public:
    void onPlaybackEvent(PlaybackEvent event, int user_id, int song_id) override {
        if (event == PlaybackEvent::START) {
            cout << "[ANALYTICS] Stream started -> User: " << user_id 
                 << " | Track: " << song_id << " (Published to TimeSeries DB)\n";
        }
    }
};

class UserHistoryService : public IPlaybackObserver {
public:
    void onPlaybackEvent(PlaybackEvent event, int user_id, int song_id) override {
        if (event == PlaybackEvent::START) {
            cout << "[USER COMPONENT] Appending Track: " << song_id 
                 << " to listening history for User: " << user_id << "\n";
        }
    }
};

// ============================================================================
// 4. STRATEGY PATTERN (INFRASTRUCTURE LAYER)
// ============================================================================

class ICdnResolver {
public:
    virtual ~ICdnResolver() = default;
    virtual string getCdnUrl(const string& media_key, int expiration) const = 0;
};

class S3UrlResolver : public ICdnResolver {
private:
    string bucket;
    string aws_region;
public:
    S3UrlResolver(string b, string r) : bucket(move(b)), aws_region(move(r)) {}
    string getCdnUrl(const string& media_key, int expiration) const override {
        return "https://" + bucket + ".s3." + aws_region + ".amazonaws.com/" + media_key + "?Expires=" + to_string(expiration);
    }
};

// ============================================================================
// 5. DATA LAYER (REPOSITORIES) WITH CLEANUP DESTRUCTORS
// ============================================================================

class ISongRepository {
public:
    virtual ~ISongRepository() = default;
    virtual const Song* findById(int id) const = 0;
    virtual void Save(Song* song) = 0;
    virtual void Delete(int id) = 0;
};

class IPlaylistRepository {
public:
    virtual ~IPlaylistRepository() = default;
    virtual const Playlist* findById(int id) const = 0;
    virtual void Save(Playlist* playlist) = 0;
    virtual void Delete(int id) = 0;
};

class InMemorySongRepository : public ISongRepository {
private:
    unordered_map<int, Song*> db;
public:
    ~InMemorySongRepository() {
        for (auto& pair : db) {
            delete pair.second; // Cleans up data memory leaks
        }
    }
    const Song* findById(int id) const override {
        auto it = db.find(id);
        if (it == db.end()) return nullptr;
        return it->second;
    }
    void Save(Song* song) override { 
        if (db.count(song->getId())) {
            delete db[song->getId()];
        }
        db[song->getId()] = song; 
    }
    void Delete(int id) override { 
        auto it = db.find(id);
        if (it != db.end()) {
            delete it->second;
            db.erase(it);
        }
    }
};

class InMemoryPlaylistRepository : public IPlaylistRepository {
private:
    unordered_map<int, Playlist*> db;
public:
    ~InMemoryPlaylistRepository() {
        for (auto& pair : db) {
            delete pair.second;
        }
    }
    const Playlist* findById(int id) const override {
        auto it = db.find(id);
        if (it == db.end()) return nullptr;
        return it->second;
    }
    void Save(Playlist* playlist) override { 
        if (db.count(playlist->getId())) {
            delete db[playlist->getId()];
        }
        db[playlist->getId()] = playlist; 
    }
    void Delete(int id) override { 
        auto it = db.find(id);
        if (it != db.end()) {
            delete it->second;
            db.erase(it);
        }
    }
};

// ============================================================================
// 6. DECOUPLED CORE SERVICES (REPLACING THE GOD OBJECT)
// ============================================================================

class ManifestPackager {
private:
    const ICdnResolver* cdnResolver; // Weak reference, does not own
public:
    ManifestPackager(const ICdnResolver* cdn) : cdnResolver(cdn) {}

    string buildMasterManifestUrl(const Song& song) const {
        return cdnResolver->getCdnUrl(song.getMasterManifestKey(), 3600);
    }
};

class PlaybackQueue {
private:
    vector<int> track_ids;
    size_t current_index = 0;
public:
    void loadTracks(vector<int> ids, bool shuffle_enabled) {
        track_ids = move(ids);
        current_index = 0;
        if (shuffle_enabled) {
            random_device rd;
            mt19937 g(rd());
            shuffle(track_ids.begin(), track_ids.end(), g);
        }
    }
    optional<int> getCurrentTrack() const {
        if (current_index >= track_ids.size()) return nullopt;
        return track_ids[current_index];
    }
    void next() { current_index++; }
};

class LeanPlaybackService {
private:
    const ISongRepository* songRepo;
    const IPlaylistRepository* playlistRepo;
    const ManifestPackager* packager;
    PlaybackQueue* activeQueue;               // Owned by service
    vector<IPlaybackObserver*> observers;    // Observed references, not owned

    void notifyObservers(PlaybackEvent event, int user_id, int song_id) {
        for (auto* observer : observers) {
            observer->onPlaybackEvent(event, user_id, song_id);
        }
    }

public:
    LeanPlaybackService(const ISongRepository* sr, const IPlaylistRepository* pr, const ManifestPackager* mp)
        : songRepo(sr), playlistRepo(pr), packager(mp), activeQueue(new PlaybackQueue()) {}

    ~LeanPlaybackService() {
        delete activeQueue;
    }

    void registerObserver(IPlaybackObserver* observer) {
        observers.push_back(observer);
    }

    void playPlaylist(int user_id, int playlist_id) {
        const Playlist* playlist = playlistRepo->findById(playlist_id);
        if (!playlist) {
            cout << "[PLAYER ERROR] Playlist ID " << playlist_id << " not found.\n";
            return;
        }

        activeQueue->loadTracks(playlist->getSongIds(), playlist->shouldShuffle());
        auto currentTrackId = activeQueue->getCurrentTrack();

        if (!currentTrackId) {
            cout << "[PLAYER WARNING] Target queue is empty.\n";
            return;
        }

        const Song* song = songRepo->findById(*currentTrackId);
        if (!song) return;

        string execution_url = packager->buildMasterManifestUrl(*song);
        cout << "[ENGINE] Mounting media stream source -> " << execution_url << "\n";

        notifyObservers(PlaybackEvent::START, user_id, song->getId());
    }
};

// ============================================================================
// 6.5 SONG SEARCH SERVICE (HNSW API INNER CALLS)
// ============================================================================
class SongSearchService {
private:
    const ISongRepository* songRepo;
public:
    SongSearchService(const ISongRepository* repo) : songRepo(repo) {
        // Initialize the HNSW search engine
        HNSWEngine::Initialize("../SearchEngine", 3072, 4, 16);
    }

    void searchByRandomVector(int songId) {
        const Song* source_song = songRepo->findById(songId);
        if (!source_song) {
            cout << "[SEARCH SERVICE ERROR] Song ID " << songId << " does not exist in repository.\n";
            return;
        }

        // Generate a random vector of 3072 dimensions
        vector<float> random_query(3072);
        random_device rd;
        mt19937 rng(rd());
        uniform_real_distribution<float> dist(0.0f, 1.0f);
        for (int i = 0; i < 3072; ++i) {
            random_query[i] = dist(rng);
        }

        cout << "[SEARCH SERVICE] User requested search for Song ID " << songId 
             << " ('" << source_song->getName() << "') using a random query vector...\n";

        // Query HNSWEngine
        vector<string> results = HNSWEngine::Search(random_query, 5, 32);

        cout << "[SEARCH SERVICE] Top nearest songs found in the database:\n";
        for (size_t i = 0; i < results.size(); ++i) {
            cout << "  Match #" << i + 1 << ": " << results[i] << "\n";
        }
    }
};

// ============================================================================
// 7. EXECUTION RUNTIME WITH MANUAL DEPENDENCY CLEANUP
// ============================================================================

int main() {
    // 1. Setup Infrastructure and Repositories (Owners of Data)
    ISongRepository* songRepo = new InMemorySongRepository();
    IPlaylistRepository* playlistRepo = new InMemoryPlaylistRepository();
    ICdnResolver* cdnResolver = new S3UrlResolver("spotify-cdn-core", "us-east-1");
    
    // 2. Setup Internal Sub-Components
    ManifestPackager* packager = new ManifestPackager(cdnResolver);
    MediaFactory* mediaFactory = new StandardMediaFactory();

    // 3. Track ingestion via Factory. Repositories absorb ownership.
    Song* song1 = mediaFactory->createTrack(1, 404, Genre::METAL, "Blackened", "manifest_node_1");
    Song* song2 = mediaFactory->createTrack(2, 404, Genre::POP, "Style", "manifest_node_2");
    
    songRepo->Save(song1);
    songRepo->Save(song2);

    // 4. Configure Playlist State
    Playlist* playlist = new Playlist(10, 501);
    playlist->addSong(1);
    playlist->addSong(2);
    playlist->setShuffle(true);
    playlistRepo->Save(playlist); // Repository takes ownership

    // 5. Instantiate Core Playback System
    LeanPlaybackService* playbackService = new LeanPlaybackService(songRepo, playlistRepo, packager);

    // 6. Attach plug-and-play Observers
    IPlaybackObserver* analyticsWorker = new AudioAnalyticsService();
    IPlaybackObserver* historyWorker = new UserHistoryService();
    
    playbackService->registerObserver(analyticsWorker);
    playbackService->registerObserver(historyWorker);

    // 7. Initialize stream
    cout << "--- Initializing User Playlist Playback Stream ---\n";
    playbackService->playPlaylist(501, 10);

    // 8. Initialize and test Search Service
    cout << "\n--- Initializing Song Search Service ---\n";
    SongSearchService* searchService = new SongSearchService(songRepo);
    searchService->searchByRandomVector(1); // Test random vector query for Song ID 1
    delete searchService;

    // ========================================================================
    // MANUAL CLEANUP (Deterministic destruction to avoid any leaks)
    // ========================================================================
    delete playbackService;
    delete historyWorker;
    delete analyticsWorker;
    delete mediaFactory;
    delete packager;
    delete cdnResolver;
    
    // Repositories automatically clear their stored entities via internal destructors
    delete playlistRepo;
    delete songRepo;

    return 0;
}
#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;
typedef long long ll;

const ll INF = 1e18;
int n, k;
vector<ll> v, pre, wpre;
vector<vector<ll>> cost_matrix;

/**
 * 1D DP for the Aliens Trick.
 * Finds the minimum cost to cover all houses given a specific 'penalty' per school.
 * Returns a pair: {total_cost_with_penalties, number_of_schools_used}
 */
pair<ll, int> solve_aliens(ll penalty) {
    // dp[i] = min cost to cover houses 1 through i
    vector<ll> dp(n + 1, INF);
    
    // cnt[i] = the number of schools built to achieve dp[i]
    vector<int> cnt(n + 1, 0);
    
    dp[0] = 0;
    
    for (int i = 1; i <= n; i++) {
        for (int j = 0; j < i; j++) {
            ll current_cost = dp[j] + cost_matrix[j + 1][i] + penalty;
            
            if (current_cost < dp[i]) {
                dp[i] = current_cost;
                cnt[i] = cnt[j] + 1;
            } 
            else if (current_cost == dp[i]) {
                cnt[i] = max(cnt[i], cnt[j] + 1);
            }
        }
    }
    
    return {dp[n], cnt[n]};
}

void solve() {
    cin >> n >> k;
    
    v.assign(n + 1, 0);
    pre.assign(n + 1, 0);
    wpre.assign(n + 1, 0);
    
    for (int i = 1; i <= n; i++) {
        cin >> v[i];
        pre[i] = pre[i - 1] + v[i];
        wpre[i] = wpre[i - 1] + (v[i] * i);
    }
    
    cost_matrix.assign(n + 1, vector<ll>(n + 1, 0));
    
    for (int l = 1; l <= n; l++) {
        int m = l; 
        
        for (int r = l; r <= n; r++) {
            ll total = pre[r] - pre[l - 1];
            while (m < r) {
                ll left = pre[m] - pre[l - 1];
                if (left * 2 >= total) break; 
                m++;
            }
            ll cost_left = (ll)m * (pre[m] - pre[l - 1]) - (wpre[m] - wpre[l - 1]);
            ll cost_right = (wpre[r] - wpre[m]) - (ll)m * (pre[r] - pre[m]);
            
            cost_matrix[l][r] = cost_left + cost_right;
        }
    }
    
    ll low = 0, high = 1e15; 
    ll best_ans = INF;
    
    while (low <= high) {
        ll mid_penalty = low + (high - low) / 2;
        
        pair<ll, int> result = solve_aliens(mid_penalty);
        ll total_cost_with_penalty = result.first;
        int schools_used = result.second;
        
        if (schools_used >= k) {
            best_ans = total_cost_with_penalty - (k * mid_penalty);
            low = mid_penalty + 1;
        } else {
            high = mid_penalty - 1;
        }
    }
    
    cout << best_ans << "\n";
}

int main() {
    ios_base::sync_with_stdio(false);
    cin.tie(NULL);
    
    solve();
    
    return 0;
}
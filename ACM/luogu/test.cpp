//
// Created by hcl on 25-9-3.
//
#include <bits/stdc++.h>
using namespace std;

#define int long long
#define PII pair<int, int>
#define VI vector<int>
#define endl "\n"

inline int read() {
    char ch = getchar();
    int x = 0, f = 1;
    while (ch < '0' || ch > '9') {
        if (ch == '-') f  =-1;
        ch = getchar();
    }
    while (ch >= '0' && ch <= '9') {
        x = (x << 1) + (x << 3) + (ch ^ '0');
        ch = getchar();
    }
    return x * f;
}

const int MAXN = 2e5 + 5;
const int INF = 0x3f3f3f3f;

int sv() {
    cout << __cplusplus << endl;

    return 0;
}

signed main() {
#ifdef __LOCAL
    freopen("in.txt", "r", stdin);
#endif

    ios::sync_with_stdio(0);
    cin.tie(0);
    cout.tie(0);

    int t = 1;
    // cin >> t;
    t = read();
    while (t--) {
        sv();
    }

    return 0;
}
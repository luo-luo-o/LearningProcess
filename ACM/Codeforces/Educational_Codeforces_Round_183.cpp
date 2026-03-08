//
// Created by hcl on 25-10-6.
//

// https://codeforces.com/contest/2145

#include <bits/stdc++.h>
using namespace std;

#define int long long
#define PII pair<int, int>
#define VI vector<int>
#define endl "\n"

inline int read();

const int MAXN = 2e5 + 5;
const int INF = 0x3f3f3f3f;

// int svA() {
//     int n = read();
//     cout << (3 - n % 3) % 3  << endl;
//
//     return 0;
// }
//
// int svB() {
//
//
//     int n, k;
//     cin >> n >> k;
//     string str;
//     cin >> str;
//
//     vector<char> li(n + 1);
//     for (int i = 1; i < n + 1 ;i ++) {
//         li[i] = '+';
//     }
//     int top = 1, bottom = n;
//     int cnt = 0;
//
//     for (int i = 0; i < k ;i ++) {
//         char ch = str[i] ;
//         if (ch == '0') {
//             li[top] = '-';
//             top ++;
//         }
//         else if (ch == '1') {
//             li[bottom] = '-';
//             bottom --;
//         }
//         else if (ch == '2') {
//             cnt ++;
//         }
//     }
//
//     for (int i = 0; i < cnt; i ++) {
//         li[top + i] = '?';
//     }
//     char aa;
//     if (cnt > bottom - top) {
//         aa = '-';
//     }
//     else  aa = '?';
//     for (int i = 0; i < cnt; i ++) {
//         li[top + i] = aa;
//         li[bottom - i] = aa;
//     }
//
//     for (int i = 1; i < n + 1; i ++) {
//         cout << li[i] ;
//     }
//     cout << endl;
//
//
//
//     return 0;
// }

int sv() {  // 菜鸡没做出来QAQ
    int n = read();
    vector<char> s(n);
    int si = 0;
    vector<int> a(n, 0);
    vector<int> b(n, 0);
    char ch;
    int ac = 0, ai = 0, at = 0, acm = 0;
    int bc = 0, bi = 0, bt = 0, bcm = 0;
    while (ch = getchar()) {
        if (ch == '\n') {
            break;
        }

        if (ch == 'a') {
            if (bc != 0) {
                b[bi] = bc;
                bi ++;
                bcm = max(bcm, bc);
                bc = 0;
            }

            ac ++;
            s[si] = ch;
            si ++;
            at ++;
        }
        else if (ch == 'b') {
            if (ac != 0) {
                a[ai] = ac;
                ai ++;
                acm = max(acm, ac);
                ac = 0;
            }

            bc ++;
            s[si] = ch;
            si ++;
            bt ++;
        }
    }

    int ans = INF;
    if (at == bt) {
        cout << 0 << endl;
        return 0;
    }
    else if (at > bt) {
        if (acm >= at - bt) {
            cout << at - bt << endl;
            return 0;
        }
        else {
            ans = -1;
        }
    }
    else if (at < bt) {
        if (bcm >= bt - at) {
            cout << bt - at << endl;
            return 0;
        }
        else {
            ans = -1;
        }
    }

    if (ans == -1) {
        if (s[0] == 'a') {
            if (b[0] > a[0]) {
                cout << n - 2 * a[0] << endl;
                return 0;
            }

        }
        else if (s[0] == 'b') {
            if (a[0] > b[0]) {
                cout << n - 2 * b[0] << endl;
                return 0;
            }

        }

        if (s[n - 1] == 'a') {
            if (b[bi - 1] > a[ai - 1]) {
                cout << n - 2 * a[ai - 1] << endl;
                return 0;
            }
        }
        else if (s[n - 1] == 'b') {
            if (a[ai - 1] > b[bi - 1]) {
                cout << n - 2 * b[bi - 1] << endl;
                return 0;
            }
        }

        cout << -1 << endl;
    }


    return 0;
}

signed main() {
#ifdef __LOCAL
    if (freopen("..\\in.txt", "r", stdin) == nullptr) {
        cout << "File Error!! \"in.txt\" cannot found!!" << endl;
        return 1;
    }
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


/*

贴一个C题由deepseek生成的正确代码QAQ（已经经过测评机验证）

#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <climits>

using namespace std;

int main() {
    ios_base::sync_with_stdio(false);
    cin.tie(nullptr);

    int t;
    cin >> t;

    vector<int> results;

    while (t--) {
        int n;
        cin >> n;
        string s;
        cin >> s;

        int count_a = 0, count_b = 0;
        for (char c : s) {
            if (c == 'a') count_a++;
            else count_b++;
        }

        // 如果a和b数量已经相等，不需要删除
        if (count_a == count_b) {
            results.push_back(0);
            continue;
        }

        // 计算需要达到的差值
        int diff = count_a - count_b;

        unordered_map<int, int> prefix_pos;
        prefix_pos[0] = 0; // 前缀和为0的位置是0

        int current_sum = 0;
        int min_remove = INT_MAX;

        for (int i = 1; i <= n; i++) {
            // 更新当前前缀和
            if (s[i - 1] == 'a') {
                current_sum++;
            } else {
                current_sum--;
            }

            // 查找是否存在满足条件的前缀位置
            int target = current_sum - diff;
            if (prefix_pos.find(target) != prefix_pos.end()) {
                int length = i - prefix_pos[target];
                if (length < min_remove) {
                    min_remove = length;
                }
            }

            // 记录当前前缀和的最新位置
            prefix_pos[current_sum] = i;
        }

        // 如果找到的删除长度小于整个字符串，输出结果；否则输出-1
        if (min_remove < n) {
            results.push_back(min_remove);
        } else {
            results.push_back(-1);
        }
    }

    // 输出所有结果
    for (int res : results) {
        cout << res << "\n";
    }

    return 0;
}


 */
//
// Created by hcl on 25-9-3.
//
#include <bits/stdc++.h>
using namespace std;

#define int long long
#define PII pair<int, int>
#define VI vector<int>
#define endl "\n"
#define YES puts("Yes")
#define NO puts("No")

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

struct NODE {
    int data;
    NODE *l = nullptr, *r = nullptr;
};

struct TREE {
    NODE* root = nullptr;
} * tree;

NODE* newnode(int val) {
    NODE* node = new NODE;
    node->data = val;
    return node;
}

NODE* insert(NODE* node, int val) {
    if (node == nullptr) {
        return newnode(val);
    }
    if (node->data < val) {
        node->r = insert(node->r, val);
    }
    else if (node->data > val) {
        node->l = insert(node->l, val);
    }
    return node;
}

int cnt1 = 0;
bool found1 = false;
void dfs1(NODE* node, int val) {
    cnt1 ++;
    if (node == nullptr) return;
    dfs1(node->l, val);
    if (node->data == val) {
        cout << cnt1 << endl;
        found1 = true;
        return ;
    }
    if (!found1)
        dfs1(node->r, val);

}

void op1(int x) {
    cnt1 = 0;
    found1 = false;
    dfs1(tree->root, x);
}

void op2(int x) {

}

void op3(int x) {

}

void op4(int x) {

}

void op5(int x) {
    insert(tree->root, x);
}

int sv() {
    int op = read(), x = read();
    if (op == 1) {
        op1(x);
    }
    else if (op == 2) {
        op2(x);
    }
    else if (op == 3) {
        op3(x);
    }
    else if (op == 4) {
        op4(x);
    }
    else if (op == 5) {
        op5(x);
    }
    else {
        ;
    }

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
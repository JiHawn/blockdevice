#include <iostream>
#include <cmath>

using namespace std;

unsigned long long int k;
unsigned long long int d;

int main(void) {
    int t;
    cin >> t;
    for(int i=0; i<t; i++) {
        d = 0;
        int n;
        cin >> n;
        cin >> k;
        int disk = 1;
        unsigned long long int tmp = k;
        while(tmp%2 != 1) {
            tmp = tmp/2;
            disk++;
        }
        unsigned long long int count = k >> disk;
        count = count % 3;
        if(n%2 == 0) {
            if(disk%2 == 0) {
                if(count == 0) cout << 1 << " " << 3 << endl;
                else if (count == 1) cout << 3 << " " << 2 << endl;
                else if (count == 2) cout << 2 << " " << 1 << endl;
            } else {
                if(count == 0) cout << 1 << " " << 2 << endl;
                else if (count == 1) cout << 2 << " " << 3 << endl;
                else if (count == 2) cout << 3 << " " << 1 << endl;
            }    
        } else {
            if(disk%2 == 0) {
                if(count == 0) cout << 1 << " " << 2 << endl;
                else if (count == 1) cout << 2 << " " << 3 << endl;
                else if (count == 2) cout << 3 << " " << 1 << endl;
            } else {
                if(count == 0) cout << 1 << " " << 3 << endl;
                else if (count == 1) cout << 3 << " " << 2 << endl;
                else if (count == 2) cout << 2 << " " << 1 << endl;
            }
        }
    }
}
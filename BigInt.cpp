#include <iostream>
#include <vector>
#include <string>
#include <bitset>
#include <algorithm>
#include<algorithm>  
#include<iterator> 
#include <deque>


class BigInt {
public:
    BigInt() {
    }
    BigInt(unsigned int num) {
        blocks.push_back(num);
    }
    //copy const
    BigInt(const BigInt& other){
        this->blocks = other.blocks;
    } 
    //move const
    BigInt(BigInt&& other){
        this->blocks.clear();
        copy(other.blocks.begin(), other.blocks.end(), back_inserter(this->blocks));  
    }
    

    void setHex(const std::string& hexStr) {
        blocks.clear();
        for (size_t i = hexStr.length() - 1; i >= 0; i -= 8) {
            size_t chunkStart = (i >= 8) ? (i - 7) : 0;
            std::string chunk = hexStr.substr(chunkStart, i - chunkStart + 1);
            blocks.push_back(std::stoul(chunk, 0, 16));
            if (chunkStart == 0)
                break;
        }
        delete_empty_blocks();

    }
    std::string getHex() const {
        std::string hexStr;
        for (int i = blocks.size() - 1; i >= 0; --i) {
            char chunkHex[9];
            std::sprintf(chunkHex, "%08X", blocks[i]);
            hexStr += chunkHex;
        }
        return hexStr;
    }
    void setBin(const std::string& binStr) {
        blocks.clear();
        size_t start = 0;
        for (size_t i = binStr.length() - 1; i >= 0; i -= 32) {
            size_t chunkStart = (i >= 32) ? (i - 31) : 0;
            std::string chunk = binStr.substr(chunkStart, i - chunkStart + 1);
            blocks.push_back(std::stoul(chunk, 0, 2));
            if (chunkStart == 0)
                break;
        }
        delete_empty_blocks();
    }
    std::string getBin() const {
        std::string binStr;
        for (int i = blocks.size() - 1; i >= 0; --i) {
            std::bitset<32> binChunk(blocks[i]);
            binStr += binChunk.to_string();
        }
        return binStr;
    }
    //bits operations
    
    //INV
    BigInt operator~() {
        BigInt result(*this);
        if (result.blocks.size() == 1){
            if(result.blocks.back() == 0){
                result.blocks.back() = 1;
                return result;
            }
            result._INV_for_last_block();
            return result;
        }
        for (size_t i = 0; i < result.blocks.size() - 1; ++i) {
            result.blocks[i] = ~result.blocks[i];
        }
        result._INV_for_last_block();
        result.delete_empty_blocks();
        return result;
    }
    //xor
    BigInt operator^ (const BigInt& other) const{
        BigInt result;
        size_t m = blocks.size();
        size_t n = other.blocks.size();
        size_t maxSize = std::max(m, n);
        for (int i = 0; i < maxSize; ++i) {
            if (i > m - 1){
                result.blocks.push_back(0 ^ other.blocks[i]);
                continue;   
            }
            if (i > n - 1){
                result.blocks.push_back(blocks[i] ^ 0);
                continue;
            }
            result.blocks.push_back(blocks[i] ^ other.blocks[i]);
        }
        result.delete_empty_blocks();
        return result;
    }

    //or
    BigInt operator| (const BigInt& other){
        BigInt result; 
        size_t maxSize = std::max(blocks.size(), other.blocks.size());
        if (blocks.size() < other.blocks.size()) {
            blocks.resize(other.blocks.size(), 0);
        }
        for (size_t i = 0; i < maxSize; ++i) {
            if (i < other.blocks.size()) {
                result.blocks.push_back(blocks[i] | other.blocks[i]);
            }
        }
        delete_empty_blocks();
        result.delete_empty_blocks();
        return result;
    }

    //and
    BigInt operator& (const BigInt& other) const{
        BigInt result;
        size_t minSize = std::min(blocks.size(), other.blocks.size());
        for (size_t i = 0; i < minSize; ++i) {
            result.blocks.push_back(blocks[i] & other.blocks[i]);
        }
        result.delete_empty_blocks();
        return result;
    }
    //shift left
    BigInt operator<< (int n) {
        BigInt result(*this); 
        if (n <= 0) {
            return result; 
        }
        if (n>= 32){
            result = *this << (n%32);
            for(int i = 1; i <= n / 32; i++)
                result.blocks.push_front(0);
            return result;
        }
        int carry = 0;
        for (int i = 0; i < result.blocks.size(); ++i) {
            unsigned int shifted = result.blocks[i] << n;
            shifted |= carry;  
            carry = result.blocks[i] >> (32 - n);
            result.blocks[i] = shifted;
        }
        if (carry != 0) {
            result.blocks.push_back(carry);
        }
        result.delete_empty_blocks();
        return result;
    }
    
    //shift right
    BigInt operator>> (int n) {
        BigInt result(*this); 
        if (n <= 0) {
            return result;
        }
        if (n>= 32){
            for(int i = 1; i <= n / 32; i++)
                result.blocks.pop_front();
            return result >> (n % 32);
        }

        int carry = 0;
        for (int i = result.blocks.size() - 1; i >= 0; --i) {
            unsigned int shifted = result.blocks[i] >> n;
            shifted |= carry << (32 - n); 
            carry = (result.blocks[i] & ((1 << n) - 1)); 
            result.blocks[i] = shifted;
        }
        result.delete_empty_blocks();
        return result;
    }

    //arifmetical operations
    //add
    BigInt operator+(const BigInt& other) const{
        BigInt result;
        unsigned int carry = 0;
        size_t max_size = std::max(blocks.size(), other.blocks.size());
        for (size_t i = 0; i < max_size || carry; ++i) {
            if (i == result.blocks.size()) {
                result.blocks.push_back(0);
            }
            unsigned long long sum = result.blocks[i];
            if (i < blocks.size()) {
                sum += blocks[i];
            }
            if (i < other.blocks.size()) {
                sum += other.blocks[i];
            }
            sum += carry;
            result.blocks[i] = static_cast<unsigned int>(sum);
            carry = static_cast<unsigned int>(sum >> 32);
        }
        result.delete_empty_blocks();
        return result;
    }
    // subtitude
    BigInt operator- (const BigInt& other) const{
        BigInt result;
        size_t max_size = blocks.size();
        result.blocks.resize(max_size);

        unsigned long long borrow = 0;
        for (size_t i = 0; i < max_size; ++i) {
            long long diff = static_cast<unsigned long long>(blocks[i]);
            if (i < other.blocks.size()) {
                diff -= static_cast<unsigned long long>(other.blocks[i]);
            }
            diff -=borrow;

            if (diff < 0){
                diff += (1ULL << 32);    
                borrow = 1;
            }
            else{
                borrow = 0;
            }
            result.blocks[i] = static_cast<unsigned int>(diff);
        }

        result.delete_empty_blocks();

        return result;
    }
    //mod
    unsigned int operator% (unsigned int mod) const{
        unsigned int result = 0;
        for (int i = blocks.size() - 1; i >= 0; --i) {
            for(int j = 7; j >= 0; --j) {
                result = (result * 16 + ((blocks[i] >> (4*j)) & 0xF)) % mod;
            }
        }
        return result;
    }
    //other operator overload
    BigInt& operator=(const BigInt& other){
        if (this != &other) {
            blocks = other.blocks;
        }
        return *this;
    }
    bool operator< (const BigInt& other) const{
        if (&other == nullptr)
            return false;
        if (blocks.size() < other.blocks.size())
            return true;
        if (blocks.size() > other.blocks.size())
            return false;
        for (size_t i = 0; i <blocks.size(); i++){
            if(blocks[i] < other.blocks[i])
                return true;
            if(blocks[i] > other.blocks[i])
                return false;
        }
        return false;
    }
    bool operator> (const BigInt& other) const{
        if (&other == nullptr)
            return false;
        if (blocks.size() > other.blocks.size())
            return true;
        if (blocks.size() < other.blocks.size())
            return false;
        for (size_t i = 0; i <blocks.size(); i++){
            if(blocks[i] > other.blocks[i])
                return true;
            if(blocks[i] < other.blocks[i])
                return false;
        }
        return false;
    }
    bool operator== (const BigInt& other) const{
        if (&other == nullptr)
            return false;
        if (blocks.size() != other.blocks.size())
            return false;
        for (size_t i = 0; i <blocks.size(); i++){
            if(blocks[i] != other.blocks[i])
                return false;
        }
        return true;
    }
    bool operator>= (const BigInt& other) const{
        if((*this) > other || (*this) == other)
            return true;
        return false;
    }
    
    bool operator<= (const BigInt& other) const{
        if((*this) < other || (*this) == other)
            return true;
        return false;
    }
    //mul
    BigInt operator* (const BigInt& other) const {
        size_t n = blocks.size();
        size_t m = other.blocks.size();

        if (n == 0 || m == 0) {
            return BigInt(0);
        }

        if (n == 1 || m == 1) {
            return MultiplyBySingleBlock(other);
        }

        size_t maxLen = std::max(n, m);
        size_t halfLen = maxLen / 2;

        BigInt a0, a1, b0, b1;
        SplitInHalf(a0, a1, halfLen);
        other.SplitInHalf(b0, b1, halfLen);

        BigInt p1 = a1*b1;
        BigInt p2 = a0*b0;
        BigInt p3 = (a0 + a1)*(b0 + b1) - p1 - p2;

        p1 << (halfLen * 2 * 32);
        p3 << (halfLen * 32);
        return p1 + p2 + p3;
    }


private:
    void SplitInHalf(BigInt& high, BigInt& low, size_t halfLen) const {
        if (halfLen < blocks.size()) {
            high.blocks.assign(blocks.begin(), blocks.begin() + halfLen);
            low.blocks.assign(blocks.begin() + halfLen, blocks.end());
        } else {
            high = BigInt(0);
            low = *this;
        }
    }

    BigInt MultiplyBySingleBlock(const BigInt& other) const {
        if (blocks.empty() || other.blocks.empty()) {
            return BigInt(0);
        }
        if ((*this) < other){
            return other.MultiplyBySingleBlock(*this);
        }
        BigInt result;
        result.blocks.resize(blocks.size() + other.blocks.size(), 0);

        unsigned long long carry = 0;

        for (size_t i = 0; i < blocks.size(); ++i) {
            unsigned long long product = static_cast<unsigned long long>(blocks[i]) * other.blocks[0];
            product += carry;
            product += result.blocks[i];
            result.blocks[i] = static_cast<unsigned int>(product);
            carry = product >> 32;
        }

        if (carry > 0) {
            result.blocks[blocks.size()] = static_cast<unsigned int>(carry);
        }
        result.delete_empty_blocks();
        return result;
    }
    void _INV_for_last_block(){
        unsigned int lastBlock = blocks.back();
        int leadingZeros = __builtin_clz(lastBlock);
        if (leadingZeros < 32) {
            lastBlock ^= ((1u << (32 - leadingZeros)) - 1);
        }
        blocks.back() = lastBlock;
    }
    void delete_empty_blocks(){
        while (!blocks.empty() && blocks.back() == 0 && blocks.size() != 1) {
            blocks.pop_back();
        }
    }
    std::deque<unsigned int> blocks;
};


//functions for answer check
std::string make_correct_len(std::string str, const int base){
    int mod = (str).length() % base;
    if(mod != 0)
        (str).insert(0, base - mod, '0');
    std::transform(str.begin(), str.end(), str.begin(), ::toupper);
    return str;

}
bool compare (std::string ans, std::string result){
    return ans == result;
}

int main(){
    BigInt num, num2, num3;
    std::string inp;
    std::string out;
    std::cout<<"\n\n\n Test from document: \n\n\n"<<std::endl;

    std::cout <<"For XOR:"<< std::endl;    
    num.setHex("51bf608414ad5726a3c1bec098f77b1b54ffb2787f8d528a74c1d7fde6470ea4");
    num2.setHex("403db8ad88a3932a0b7e8189aed9eeffb8121dfac05c3512fdb396dd73f6331c");
    out = "1182d8299c0ec40ca8bf3f49362e95e4ecedaf82bfd167988972412095b13db8";
    std::cout << "Bin1: " << num.getHex() << std::endl;
    std::cout << "Bin2: " << num2.getHex() << std::endl;
    std::cout <<"----"<< std::endl;
    num3 = num ^ num2;
    out = make_correct_len(out, 8);
    std::cout << "Expect: " << out<<std::endl;
    std::cout << "Result: " << num3.getHex() << std::endl;
    std::cout <<"Correct : " <<compare(out, num3.getHex())<<std::endl;
    std::cout <<"_________________________"<< std::endl;

    std::cout <<"For ADD:"<< std::endl;    
    num.setHex("36f028580bb02cc8272a9a020f4200e346e276ae664e45ee80745574e2f5ab80");
    num2.setHex("70983d692f648185febe6d6fa607630ae68649f7e6fc45b94680096c06e4fadb");
    out = "a78865c13b14ae4e25e90771b54963ee2d68c0a64d4a8ba7c6f45ee0e9daa65b";
    std::cout << "Bin1: " << num.getHex() << std::endl;
    std::cout << "Bin2: " << num2.getHex() << std::endl;
    std::cout <<"----"<< std::endl;
    num3 = num + num2;
    out = make_correct_len(out, 8);
    std::cout << "Expect: " << out<<std::endl;
    std::cout << "Result: " << num3.getHex() << std::endl;
    std::cout <<"Correct : " <<compare(out, num3.getHex())<<std::endl;
    std::cout <<"_________________________"<< std::endl;

    
    std::cout <<"For SUB:"<< std::endl;    
    num.setHex("33ced2c76b26cae94e162c4c0d2c0ff7c13094b0185a3c122e732d5ba77efebc");
    num2.setHex("22e962951cb6cd2ce279ab0e2095825c141d48ef3ca9dabf253e38760b57fe03");
    out = "10e570324e6ffdbc6b9c813dec968d9bad134bc0dbb061530934f4e59c2700b9";
    std::cout << "Bin1: " << num.getHex() << std::endl;
    std::cout << "Bin2: " << num2.getHex() << std::endl;
    std::cout <<"----"<< std::endl;
    num3 = num - num2;
    out = make_correct_len(out, 8);
    std::cout << "Expect: " << out<<std::endl;
    std::cout << "Result: " << num3.getHex() << std::endl;
    std::cout <<"Correct : " <<compare(out, num3.getHex())<<std::endl;
    std::cout <<"_________________________"<< std::endl;

    
    std::cout <<"For MUL:"<< std::endl;    
    num.setHex("7d7deab2affa38154326e96d350deee1");
    num2.setHex("97f92a75b3faf8939e8e98b96476fd22");
    out = "4a7f69b908e167eb0dc9af7bbaa5456039c38359e4de4f169ca10c44d0a416e2";
    std::cout << "Bin1: " << num.getHex() << std::endl;
    std::cout << "Bin2: " << num2.getHex() << std::endl;
    std::cout <<"----"<< std::endl;
    num3 = num * num2;
    out = make_correct_len(out, 8);
    std::cout << "Expect: " << out<<std::endl;
    std::cout << "Result: " << num3.getHex() << std::endl;
    std::cout <<"Correct : " <<compare(out, num3.getHex())<<std::endl;
    std::cout <<"_________________________"<< std::endl;

    std::cout<<"\n\n\n Other tests: \n\n\n"<<std::endl;

    //TEST 1
    num;
    inp = "1101000000000000000000000000000000000";
    inp = make_correct_len(inp, 32);
    num.setBin(inp);
    std::cout << "Inp: "<< inp<<std::endl;
    std::cout << "Hex: " << num.getHex() << std::endl;
    std::cout << "Bin: " << num.getBin() << std::endl;
    std::cout << "Is inp equal to bin : " << compare(num.getBin(), inp) << std::endl; 
    std::cout <<"_________________________"<< std::endl;

    //TEST 2
    inp = "e035c6cfa42609b998b883bc1699df885cef74e2b2cc372eb8fa7e7";
    num.setHex(inp);
    inp = make_correct_len(inp, 8);
    std::cout << "Inp: "<< inp<<std::endl;
    std::cout << "Hex: " << num.getHex() << std::endl;
    std::cout << "Bin: " << num.getBin() << std::endl;
    std::cout << "Is inp equal to hex : " << compare(num.getHex(), inp) << std::endl; 
    std::cout <<"_________________________"<< std::endl;

    //Test3
    std::cout <<"For INV:"<< std::endl;
    num;
    inp = "101001";
    out = "10110";
    num.setBin(inp);
    std::cout<<"Before : "<<std::endl;
    std::cout << "Hex: " << num.getHex() << std::endl;
    std::cout << "Bin: " << num.getBin() << std::endl;
    std::cout <<"----"<< std::endl;
    num3 = ~num;
    std::cout<<"After : "<<std::endl;
    std::cout << "Hex: " << num3.getHex() << std::endl;
    std::cout << "Bin: " << num3.getBin() << std::endl;
    std::cout <<"----"<< std::endl;
    out = make_correct_len(out, 32);
    std::cout << "Expect: " << out<<std::endl;
    std::cout << "Result: " << num3.getBin() << std::endl;
    std::cout <<"Correct : " <<compare(out, num3.getBin())<<std::endl;
    
    std::cout <<"_________________________"<< std::endl;

    //test4
    std::cout <<"For XOR:"<< std::endl;    
    num.setBin("11010010101110101001101111101001");
    num2.setBin("10101011101001111111111111110101010111");
    out = "10101000111011010001010110011010111110";
    std::cout << "Bin1: " << num.getBin() << std::endl;
    std::cout << "Bin2: " << num2.getBin() << std::endl;
    std::cout <<"----"<< std::endl;
    num3 = num ^ num2;
    out = make_correct_len(out, 32);
    std::cout << "Expect: " << out<<std::endl;
    std::cout << "Result: " << num3.getBin() << std::endl;
    std::cout <<"Correct : " <<compare(out, num3.getBin())<<std::endl;
    std::cout <<"_________________________"<< std::endl;

    //test5
    
    std::cout <<"For OR:"<< std::endl;
    num.setBin("11010010101110101001101111101001");
    num2.setBin("10101011101001111111111111110101010111");
    out = "10101011111011111111111111111111111111";
    std::cout << "Bin1: " << num.getBin() << std::endl;
    std::cout << "Bin2: " << num2.getBin() << std::endl;
    std::cout <<"----"<< std::endl;
    num3 =  num | num2;
    out = make_correct_len(out, 32);
    std::cout << "Expect: " << out<<std::endl;
    std::cout << "Result: " << num3.getBin() << std::endl;
    std::cout <<"Correct : " <<compare(out, num3.getBin())<<std::endl;
    std::cout <<"_________________________"<< std::endl;

    //test6
    std::cout <<"For AND:"<< std::endl;
    num.setBin("11010010101110101001101111101001");
    num2.setBin("10101011101001111111111111110101010111");
    out = "11000000101110101001100101000001";
    std::cout << "Bin1: " << num.getBin() << std::endl;
    std::cout << "Bin2: " << num2.getBin() << std::endl;
    std::cout <<"----"<< std::endl;
    num3 = num & num2;
    out = make_correct_len(out, 32);
    std::cout << "Expect: " << out<<std::endl;
    std::cout << "Result: " << num3.getBin() << std::endl;
    std::cout <<"Correct : " <<compare(out, num3.getBin())<<std::endl;
    std::cout <<"_________________________"<< std::endl;

    //test 7
    std::cout <<"For shiftL by 4 #1:"<< std::endl;
    num.setBin("110101101010101110101101010101111");
    out = "0001101011010101011101011010101011110000";
    std::cout << "Bin1: " << num.getBin() << std::endl;
    std::cout <<"----"<< std::endl;
    num3 = num<<4;
    out = make_correct_len(out, 32);
    std::cout << "Expect: " << out<<std::endl;
    std::cout << "Result: " << num3.getBin() << std::endl;
    std::cout <<"Correct : " <<compare(out, num3.getBin())<<std::endl;
    std::cout <<"_________________________"<< std::endl;

    //test 8
    std::cout <<"For shiftL by 36 #2:"<< std::endl;
    num.setBin("10101110101101010101111");
    out = "010101110101101010101111000000000000000000000000000000000000";
    std::cout << "Bin1: " << num.getBin() << std::endl;
    std::cout <<"----"<< std::endl;
    num3 = num<<(36);
    out = make_correct_len(out, 32);
    std::cout << "Expect: " << out<<std::endl;
    std::cout << "Result: " << num3.getBin() << std::endl;
    std::cout <<"Correct : " <<compare(out, num3.getBin())<<std::endl;
    std::cout <<"_________________________"<< std::endl;
    
    //test 9
    std::cout <<"For shiftR by 4:"<< std::endl;
    num.setBin("111000110101101010101110101101010101111");
    out = "11100011010110101010111010110101010";
    std::cout << "Bin1: " << num.getBin() << std::endl;
    std::cout <<"----"<< std::endl;
    num3 = num>>(4);
    out = make_correct_len(out, 32);
    std::cout << "Expect: " << out<<std::endl;
    std::cout << "Result: " << num3.getBin() << std::endl;
    std::cout <<"Correct : " <<compare(out, num3.getBin())<<std::endl;
    std::cout <<"_________________________"<< std::endl;
    
    
    std::cout <<"For shiftR by 33:"<< std::endl;
    num.setBin("111000110101101010101110101101010101111");
    out = "00111000";
    std::cout << "Bin1: " << num.getBin() << std::endl;
    std::cout <<"----"<< std::endl;
    num3 = num>>33;
    out = make_correct_len(out, 32);
    std::cout << "Expect: " << out<<std::endl;
    std::cout << "Result: " << num3.getBin() << std::endl;
    std::cout <<"Correct : " <<compare(out, num3.getBin())<<std::endl;
    std::cout <<"_________________________"<< std::endl;
    
    return 0;
}

#include <iostream>
#include <vector>
#include <string>
#include <bitset>
#include <algorithm>

class BigInt {
public:
    BigInt() {
        blocks.push_back(0);
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
    void INV() {
        if (blocks.size() == 1){
            if(blocks.back() == 0){
                blocks.back() = 1;
                return;
            }
            _INV_for_last_block();
            return;
        }
        for (size_t i = 0; i < blocks.size() - 1; ++i) {
            blocks[i] = ~blocks[i];
        }
        _INV_for_last_block();
        delete_empty_blocks();
    }
    void XOR(const BigInt& other) {
        size_t minSize = std::min(blocks.size(), other.blocks.size());
        for (size_t i = 0; i < minSize; ++i) {
            blocks[i] ^= other.blocks[i];
        }
        delete_empty_blocks();
    }
    void OR(const BigInt& other) {
        size_t maxSize = std::max(blocks.size(), other.blocks.size());
        if (blocks.size() < other.blocks.size()) {
            blocks.resize(other.blocks.size(), 0);
        }
        for (size_t i = 0; i < maxSize; ++i) {
            if (i < other.blocks.size()) {
                blocks[i] |= other.blocks[i];
            }
        }
        delete_empty_blocks();
    }

    void AND(const BigInt& other) {
        size_t minSize = std::min(blocks.size(), other.blocks.size());
        blocks.resize(minSize);
        for (size_t i = 0; i < minSize; ++i) {
            blocks[i] &= other.blocks[i];
        }
        delete_empty_blocks();
    }
    void shiftL(int n) {
        if (n <= 0) {
            return; 
        }
        int carry = 0;
        for (int i = 0; i < blocks.size(); ++i) {
            unsigned int shifted = blocks[i] << n;
            shifted |= carry;  
            carry = blocks[i] >> (32 - n);
            blocks[i] = shifted;
        }
        if (carry != 0) {
            blocks.push_back(carry);
        }
        delete_empty_blocks();
    }

    void shiftR(int n) {
        if (n <= 0) {
            return;
        }
        int carry = 0;
        for (int i = blocks.size() - 1; i >= 0; --i) {
            unsigned int shifted = blocks[i] >> n;
            shifted |= carry << (32 - n); 
            carry = (blocks[i] & ((1 << n) - 1)); 
            blocks[i] = shifted;
        }
        delete_empty_blocks();
    }

    //arifmetical operations
    BigInt ADD(const BigInt& other) const {
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

    BigInt SUB(const BigInt& other) const {
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
    unsigned int MOD(unsigned int mod) {
        unsigned int result = 0;
        for (int i = blocks.size() - 1; i >= 0; --i) {
            for(int j = 7; j >= 0; --j) {
                //std::cout<<std::hex<<((blocks[i] >> (4*j)) & 0xF);
                result = (result * 16 + ((blocks[i] >> (4*j)) & 0xF)) % mod;
            }
        }
        //std::cout<<std::endl;
        return result;
    }
private:
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
    std::vector<unsigned int> blocks;
};
//functions for answer check
std::string make_correct_len(std::string str, const int base){
    int mod = (str).length() % base;
    if(mod != 0)
        (str).insert(0, base - mod, '0');
    return str;
}
bool compare (std::string ans, std::string result){
    return ans == result;
}
int main() {    
    //BigInt num2;
    //num2.setHex("22e962951cb6cd2ce279ab0e2095825c141d48ef3ca9dabf253e38760b57fe03");
    //BigInt num3= num.SUB(num2);
    //std::cout << "Hex: " << num3.getHex() << std::endl;
    /*
    BigInt num;
    std::string inp = "1101000000000000000000000000000000000";
    inp = make_correct_len(inp, 32);
    num.setBin(inp);
    std::cout << "Inp: "<< inp<<std::endl;
    std::cout << "Hex: " << num.getHex() << std::endl;
    std::cout << "Bin: " << num.getBin() << std::endl;
    std::cout << "Is inp equal to bin : " << compare(num.getBin(), inp) << std::endl; 
    std::cout <<"_________________________"<< std::endl;


    inp = "e035c6cfa42609b998b883bc1699df885cef74e2b2cc372eb8fa7e7";
    inp = make_correct_len(inp, 8);
    num.setHex(inp);
    std::transform(inp.begin(), inp.end(), inp.begin(), ::toupper);
    std::cout << "Inp: "<< inp<<std::endl;
    std::cout << "Hex: " << num.getHex() << std::endl;
    std::cout << "Bin: " << num.getBin() << std::endl;
    std::cout << "Is inp equal to hex : " << compare(num.getHex(), inp) << std::endl; 
    std::cout <<"_________________________"<< std::endl;


    std::cout <<"For INV:"<< std::endl;
    BigInt num3;
    num3.setBin("101001"); 
    std::cout << "Hex: " << num3.getHex() << std::endl;
    std::cout << "Bin: " << num3.getBin() << std::endl;
    std::cout <<"----"<< std::endl;
    num3.INV();
    std::cout << "Hex: " << num3.getHex() << std::endl;
    std::cout << "Bin: " << num3.getBin() << std::endl;
    std::cout <<"_________________________"<< std::endl;


    std::cout <<"For XOR:"<< std::endl;    
    BigInt num2;
    num2.setBin("10101000000000000000000000000000000000");
    std::cout << "Bin1: " << num.getBin() << std::endl;
    std::cout << "Bin2: " << num2.getBin() << std::endl;
    std::cout <<"----"<< std::endl;
    num.XOR(num2);
    std::cout << "XOR: " << num.getBin() << std::endl;
    std::cout <<"_________________________"<< std::endl;


    std::cout <<"For XOR:"<< std::endl;
    num.setBin("1101000000000000000000000000000000000");
    num2.setBin("10101000000000000000000000000000000000");
    std::cout << "Bin1: " << num.getBin() << std::endl;
    std::cout << "Bin2: " << num2.getBin() << std::endl;
    std::cout <<"----"<< std::endl;
    num.OR(num2);
    std::cout << " OR:  " << num.getBin() << std::endl;
    std::cout <<"_________________________"<< std::endl;


    std::cout <<"For AND:"<< std::endl;
    num.setBin("1101000000000000000000000000000000000");
    num2.setBin("10101000000000000000000000000000000000");
    std::cout << "Bin1: " << num.getBin() << std::endl;
    std::cout << "Bin2: " << num2.getBin() << std::endl;
    std::cout <<"----"<< std::endl;
    num.AND(num2);
    std::cout << "AND:  " << num.getBin() << std::endl;
    std::cout <<"_________________________"<< std::endl;

    
    std::cout <<"For shiftL #1:"<< std::endl;
    num.setBin("110101101010101110101101010101111");
    std::cout << "Bin1: " << num.getBin() << std::endl;
    std::cout <<"----"<< std::endl;
    num.shiftL(4);
    std::cout << "SFTl: " << num.getBin() << std::endl;
    std::cout <<"_________________________"<< std::endl;


    std::cout <<"For shiftL #2:"<< std::endl;
    num.setBin("10101110101101010101111");
    std::cout << "Bin1: " << num.getBin() << std::endl;
    std::cout <<"----"<< std::endl;
    num.shiftL(15);
    std::cout << "SFTl: " << num.getBin() << std::endl;
    std::cout <<"_________________________"<< std::endl;

    std::cout <<"For shiftR:"<< std::endl;
    num.setBin("111000110101101010101110101101010101111");
    std::cout << "Bin1: " << num.getBin() << std::endl;
    std::cout <<"----"<< std::endl;
    num.shiftR(4);
    std::cout << "SFTR: " << num.getBin() << std::endl;
    std::cout <<"_________________________"<< std::endl;
    
    
    std::cout <<"For shiftR:"<< std::endl;
    num.setBin("111000110101101010101110101101010101111");
    std::cout << "Bin1: " << num.getBin() << std::endl;
    std::cout <<"----"<< std::endl;
    num.shiftR(10);
    std::cout << "SFTR: " << num.getBin() << std::endl;
    std::cout <<"_________________________"<< std::endl;
    
    */
    return 0;
}

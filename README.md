# DL_labs

## Lab2 unsigned BigInteger class

This class contains one field deque<usigned int >. Each element of deque represented as 32 bits. Every hex or binary number splited by 32 bits and writes into one field deque.
To create class use:  `BigInt number;` .

To set a num use:
- `number.setHex(str)`
- `number.setBIn(str)`

To get num use:
- `number.getHex(str)`
- `number.getBIn(str)`

There are bitwise operations overrided for this datastructure:
- INV (~)
- OR  (|)
- XOR (^)
- AND (&)
- Shift Left (<<)
- Shift Right (>>)

There are arifmetical operations overrided for this datastructure:
- Add (+)
- Subtitude (-)  
- Multiplication (*)
- Module (%)

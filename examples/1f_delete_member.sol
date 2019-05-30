pragma solidity ^0.5;


contract DeleteExample {
    uint data;
    uint[] dataArray;

    function f() public {
        uint x = data;
        delete x;
        delete data;
    }
}

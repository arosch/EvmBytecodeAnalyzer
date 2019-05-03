pragma solidity ^0.5;

contract Addition {

    function add(int a, int b) public pure returns(int result){
        int c=a+b;
        return c;
    }

    function sub(int a) public pure returns (int result){
        int c = a-10;
        return c;
    }
}

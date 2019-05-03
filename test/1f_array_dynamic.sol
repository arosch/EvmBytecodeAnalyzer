pragma solidity ^0.5;

contract C {

    function f() public pure {
        uint8[] memory a = new uint8[](7);
        a[1] = 11;
        a[2] = 12;
        a[3] = 13;
        a[6] = 14;
    }

}

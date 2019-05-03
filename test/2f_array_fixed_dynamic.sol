pragma solidity ^0.5;

contract C {

    function f() public pure {
        uint[] memory a = new uint[](7);
        //assert(a.length == 7);
        //assert(b.length == len);
        a[6] = 8;
    }

    function f2() public pure {
        uint[7] memory a;
        //bytes memory b = new bytes(len);
        //assert(a.length == 7);
        //assert(b.length == len);
        a[6] = 8;
    }

}

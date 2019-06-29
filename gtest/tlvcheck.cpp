#include <gtest/gtest.h>
 
#include "../src/tlv.h"
#include "../src/errors.h"

using namespace Util;

/* tree:
 * --f4
 * ----81 0102
 * ----82 0304
 * ----7f49
 * ------85 0405
 * ------86 06
 * ------87 07
 * ----83 08090a
 * ----84 aa
 * 
 */
const auto sampletree = "\xf4\x1d\x81\x02\x01\x02\x82\x02\x03\x04\x7f\x49\x0a\x85\x02\x04\x05\x86\x01\x06\x87\x01\x07\x83\x03\x08\x09\x0a\x84\x01\xaa"_bstr;

TEST(tlvTest, Constructor) {
    auto data = "\x81\x02\x01\x02"_bstr;
    TLVTree tlv;
    
    auto err = tlv.Init(""_bstr);
    EXPECT_TRUE(err != Error::NoError);
    
    err = tlv.Init(data);
    EXPECT_TRUE(err == Error::NoError);
    EXPECT_TRUE(tlv.GetDataLink() == data);
    
    EXPECT_EQ(tlv.CurrentElm().GetPtr(), data.uint8Data());
    EXPECT_EQ(tlv.CurrentElm().Length(), 2);
    EXPECT_EQ(tlv.CurrentElm().Tag(), 0x81);
    EXPECT_EQ(tlv.CurrentElm().GetData()[0], 0x01);
    EXPECT_TRUE(tlv.CurrentElm().GetData() == "\x01\x02"_bstr);
} 

TEST(tlvTest, TreeMove) {
    TLVTree tlv;
    auto err = tlv.Init(sampletree);
    EXPECT_TRUE(err == Error::NoError);
    
    EXPECT_EQ(tlv.CurrentElm().Tag(), 0xf4);
    
    EXPECT_FALSE(tlv.GoNext()); // no next
    EXPECT_EQ(tlv.CurrentElm().Tag(), 0xf4);
    
    EXPECT_FALSE(tlv.GoParent()); // no parent
    EXPECT_EQ(tlv.CurrentElm().Tag(), 0xf4);
    
    EXPECT_TRUE(tlv.GoChild());
    EXPECT_EQ(tlv.CurrentElm().Tag(), 0x81);
    
    EXPECT_FALSE(tlv.GoChild()); // cant
    EXPECT_EQ(tlv.CurrentElm().Tag(), 0x81);
    
    EXPECT_TRUE(tlv.GoNext()); 
    EXPECT_EQ(tlv.CurrentElm().Tag(), 0x82);
    
    EXPECT_TRUE(tlv.GoNext()); 
    EXPECT_EQ(tlv.CurrentElm().Tag(), 0x7f49);
    
    EXPECT_TRUE(tlv.GoChild());
    EXPECT_EQ(tlv.CurrentElm().Tag(), 0x85);
    
    EXPECT_TRUE(tlv.GoParent());
    EXPECT_EQ(tlv.CurrentElm().Tag(), 0x7f49);
    
    EXPECT_TRUE(tlv.GoChild());
    EXPECT_EQ(tlv.CurrentElm().Tag(), 0x85);
    
    EXPECT_TRUE(tlv.GoNext()); 
    EXPECT_EQ(tlv.CurrentElm().Tag(), 0x86);
    
    EXPECT_TRUE(tlv.GoNext()); 
    EXPECT_EQ(tlv.CurrentElm().Tag(), 0x87);
    EXPECT_FALSE(tlv.CurrentElmIsLast());
    
    EXPECT_TRUE(tlv.GoParent());
    EXPECT_EQ(tlv.CurrentElm().Tag(), 0x7f49);
    
    EXPECT_TRUE(tlv.GoFirst());
    EXPECT_EQ(tlv.CurrentElm().Tag(), 0xf4);
}

TEST(tlvTest, TreeMoveNextTreeElm) {
    TLVTree tlv;
    auto err = tlv.Init(sampletree);
    EXPECT_TRUE(err == Error::NoError);
    EXPECT_EQ(tlv.CurrentElm().Tag(), 0xf4);
    
    EXPECT_FALSE(tlv.CurrentElmIsLast());
    
    EXPECT_TRUE(tlv.GoNextTreeElm());
    EXPECT_EQ(tlv.CurrentElm().Tag(), 0x81);
    
    EXPECT_TRUE(tlv.GoNextTreeElm()); 
    EXPECT_EQ(tlv.CurrentElm().Tag(), 0x82);
    
    EXPECT_TRUE(tlv.GoNextTreeElm()); 
    EXPECT_EQ(tlv.CurrentElm().Tag(), 0x7f49);
    
    EXPECT_TRUE(tlv.GoNextTreeElm()); 
    EXPECT_EQ(tlv.CurrentElm().Tag(), 0x85);

    EXPECT_TRUE(tlv.GoParent());
    EXPECT_EQ(tlv.CurrentElm().Tag(), 0x7f49);

    EXPECT_TRUE(tlv.GoNext()); 
    EXPECT_EQ(tlv.CurrentElm().Tag(), 0x83);

    EXPECT_TRUE(tlv.GoNextTreeElm()); 
    EXPECT_EQ(tlv.CurrentElm().Tag(), 0x84);
    EXPECT_TRUE(tlv.CurrentElmIsLast());

    EXPECT_FALSE(tlv.GoNextTreeElm()); // end of tree 
    EXPECT_EQ(tlv.CurrentElm().Tag(), 0x84);
}

TEST(tlvTest, TreeSearch) {
    TLVTree tlv;
    auto err = tlv.Init(sampletree);
    EXPECT_TRUE(err == Error::NoError);
    
    TLVElm *elm = tlv.Search(0x86);
    ASSERT_NE(elm, nullptr);
    EXPECT_EQ(elm->Tag(), 0x86);
    EXPECT_EQ(tlv.CurrentElm().Tag(), 0x86);

    elm = tlv.Search(0x7f49);
    ASSERT_NE(elm, nullptr);
    EXPECT_EQ(elm->Tag(), 0x7f49);
    EXPECT_EQ(tlv.CurrentElm().Tag(), 0x7f49);

    elm = tlv.Search(0x99);
    ASSERT_EQ(elm, nullptr);
    EXPECT_EQ(tlv.CurrentElm().Tag(), 0xf4);
}

TEST(tlvTest, AddRoot) {
    uint8_t _data[50] = {0};
    auto data = bstr(_data, 0, sizeof(_data));
    
    TLVTree tlv;
    auto err = tlv.Init(data);
    EXPECT_TRUE(err != Error::NoError);
    
    tlv.AddRoot(0x7f49);
    EXPECT_EQ(tlv.CurrentElm().Tag(), 0x7f49);
    EXPECT_TRUE(tlv.GetDataLink() == "\x7f\x49\x00"_bstr);
    
    auto elm = "sd"_bstr;
    tlv.AddRoot(0x7f49, &elm);
    EXPECT_EQ(tlv.CurrentElm().Tag(), 0x7f49);
    EXPECT_TRUE(tlv.GetDataLink() == "\x7f\x49\x02sd"_bstr);
}

TEST(tlvTest, AddChildAddNextDel) {
    uint8_t _data[50] = {0};
    auto data = bstr(_data, 0, sizeof(_data));
    
    TLVTree tlv;
    auto err = tlv.Init(data);
    EXPECT_TRUE(err != Error::NoError);
    
    tlv.AddRoot(0x7f49);
    EXPECT_EQ(tlv.CurrentElm().Tag(), 0x7f49);

    bstr test = "1234"_bstr;
    bstr test2 = "98"_bstr;
    tlv.AddChild(0xf4, &test);
    tlv.AddNext(0x82, &test2);
    
    tlv.Search(0xf4);
    EXPECT_EQ(tlv.CurrentElm().Tag(), 0xf4);
    EXPECT_TRUE(tlv.GetDataLink() == "\x7f\x49\x0a\xf4\x04\x31\x32\x33\x34\x82\x02\x39\x38"_bstr);
    
    tlv.AddChild(0x83);
    tlv.AddNext(0x84, &test);
    tlv.AddNext(0x85, &test2);
    
    tlv.Search(0x84);
    EXPECT_EQ(tlv.CurrentElm().Tag(), 0x84);
    
    tlv.Search(0x85);
    EXPECT_EQ(tlv.CurrentElm().Tag(), 0x85);
    EXPECT_TRUE(tlv.GetDataLink() == "\x7f\x49\x12\xf4\x0c\x83\x00\x84\x04\x31\x32\x33\x34\x85\x02\x39\x38\x82\x02\x39\x38"_bstr);

    tlv.DeleteCurrent();
    EXPECT_EQ(tlv.CurrentElm().Tag(), 0x7f49);
    EXPECT_TRUE(tlv.GetDataLink() == "\x7f\x49\x0e\xf4\x08\x83\x00\x84\x04\x31\x32\x33\x34\x82\x02\x39\x38"_bstr);
}

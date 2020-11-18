#include "pch.h"
#include "../NetCommon/net_message.h"
#include <string>

namespace Net {
	namespace {
		class HeaderTest : public ::testing::Test {
		protected:
			net::message_header<uint8_t> proper_uint8t_header;
			net::message_header<uint8_t> ill_formed_uint8t_header;

			net::message_header<std::string> proper_string_header;
			net::message_header<std::string> empty_string_header;
			std::string psh_id = "proper string header";



			HeaderTest() {
				proper_uint8t_header.id = 2;
				ill_formed_uint8t_header.id = -1;
				ill_formed_uint8t_header.size = -4;

				proper_string_header.id = "proper string header";
				proper_string_header.size = sizeof(proper_string_header);
			}

			~HeaderTest() = default;



		};

		class MessageTest : public ::testing::Test {
			protected:
				net::message<uint8_t> proper_uint8t_msg;
				net::message<std::string> proper_str_msg;
				std::size_t init_size = std::vector<uint8_t>().size() + sizeof(net::message_header<uint8_t>);

				const std::size_t remove_count = 10;
				net::message<int> remove_from_msg;

				net::message<uint8_t> ill_formed_uint8t_msg;
				net::message<std::string> ill_formed_str_msg;

				 

				MessageTest() {
					proper_uint8t_msg.header.id = 1;
					proper_str_msg.header.id = 2;

					for(auto remove_val = 0; remove_val < remove_count; remove_val++)
						remove_from_msg << remove_val;
				}

				~MessageTest() = default;
		};


		TEST_F(HeaderTest, ProperHeaderInitsTest) {
			EXPECT_EQ(proper_string_header.id, psh_id);
			EXPECT_EQ(proper_uint8t_header.id, 2);
			EXPECT_EQ(proper_string_header.size, sizeof(proper_string_header));
			EXPECT_EQ(empty_string_header.size, 0);
			EXPECT_TRUE(empty_string_header.id.empty());
		}

		TEST_F(HeaderTest, IllFormedHeaderInitsTest) {
			EXPECT_GT(ill_formed_uint8t_header.id, 0);
			EXPECT_GT(ill_formed_uint8t_header.size, 0);
		}

		TEST_F(MessageTest, SizeMethodTest) {
			std::size_t constructed_msg_size = sizeof(net::message_header<int>) + sizeof(std::vector<uint8_t>);
			std::size_t holding_data_msg_size = sizeof(net::message_header<int>) + remove_from_msg.body.size();
			ASSERT_EQ(MessageTest::ill_formed_str_msg.size(), constructed_msg_size);
			ASSERT_EQ(MessageTest::remove_from_msg.size(), holding_data_msg_size);
		}


		TEST_F(MessageTest, AddData) {
			std::vector<uint8_t> bytes = {1, 3, 5};
			std::vector<uint16_t> dbytes = {2,4,6};
			std::vector<uint32_t> qbytes = {1,3,4,6};
			std::vector<const char*> strings = {"test", "hello",""};

			using byte_type_size = std::decay<decltype(*bytes.begin())>::type;
			using dbyte_type_size = std::decay<decltype(*dbytes.begin())>::type;
			using qbyte_type_size = std::decay<decltype(*qbytes.begin())>::type;
			using str_type_size = std::decay<decltype(*strings.begin())>::type;

			
			std::size_t bytes_size = init_size + sizeof(byte_type_size)*bytes.size(),
						dbytes_size = bytes_size + sizeof(dbyte_type_size)*dbytes.size(),
						qbytes_size =  dbytes_size + sizeof(qbyte_type_size) *qbytes.size(),
						str_size =  qbytes_size + sizeof(str_type_size) * strings.size();

			EXPECT_EQ(proper_uint8t_msg.size(), init_size);

			for (auto byte : bytes) {
				proper_uint8t_msg << byte;
			}


			EXPECT_EQ(proper_uint8t_msg.size(), bytes_size);

			for (auto byte : dbytes) {
				proper_uint8t_msg << byte;
			}

			EXPECT_EQ(proper_uint8t_msg.size(), dbytes_size);
			
			for (auto byte : qbytes) {
				proper_uint8t_msg << byte;
			}

			EXPECT_EQ(proper_uint8t_msg.size(), qbytes_size);

			for (auto str : strings) {
				proper_uint8t_msg << str;
			}

			EXPECT_EQ(proper_uint8t_msg.size(), str_size);

		}


		TEST_F(MessageTest, RemoveDataTest) {
			uint8_t byte{0}; //removing 1 byte per call
			const auto start_size = remove_from_msg.size();
			for (size_t current_size = remove_count; current_size > 0; current_size--) {
				remove_from_msg >> byte;
			}
			ASSERT_EQ(remove_from_msg.size(), start_size - remove_count);

			uint16_t dbyte{0}; //removing 2 bytes per call
			for (size_t current_size = remove_count; current_size > 0; current_size--) {
				remove_from_msg >> dbyte;
			}
			ASSERT_EQ(remove_from_msg.size(), start_size - remove_count - remove_count*2);
		}

	}

	//int main(int argc, char** argv) {
	//	::testing::InitGoogleTest(&argc, argv);
	//	return RUN_ALL_TESTS();
	//}
}
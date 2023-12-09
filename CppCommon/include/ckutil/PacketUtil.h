#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <memory>

#pragma warning(push)
#pragma warning(disable: 4267)
#pragma warning(disable: 4251)


#include <google/protobuf/arena.h>


#pragma warning(pop)

#pragma warning(push)
#pragma warning(disable:4267) // size_t 변환 손실 경고 비활성화

namespace CKPacket
{
	class PacketUtil
	{
	private:
		PacketUtil() = delete;
		~PacketUtil() = delete;

	private:
		using PMessage = google::protobuf::Message*;

	public:
		static __forceinline bool IsPacket(const messageHeader& header, const std::string& str)
		{
			if (header.type().length() != str.length()) return false;

			return header.type().compare(str) == 0;
		}

		static __forceinline void Int32ToByteArr(__int32 value, char* bytes)
		{
			bytes[0] = static_cast<char>((value >> 24) & 0xFF);
			bytes[1] = static_cast<char>((value >> 16) & 0xFF);
			bytes[2] = static_cast<char>((value >> 8) & 0xFF);
			bytes[3] = static_cast<char>(value & 0xFF);
		}
		static __forceinline __int32 ByteArrToInt32(const char* bytes)
		{
			return (static_cast<__int32>(static_cast<unsigned char>(bytes[0])) << 24) |
				(static_cast<__int32>(static_cast<unsigned char>(bytes[1])) << 16) |
				(static_cast<__int32>(static_cast<unsigned char>(bytes[2])) << 8) |
				static_cast<__int32>(static_cast<unsigned char>(bytes[3]));
		}


		static __forceinline CKPacket::messageHeader* MakePackedHeader(const PMessage message)
		{
			CKPacket::messageHeader* header = new CKPacket::messageHeader();

			size_t len = message->ByteSizeLong();
			char* buf = new char[len];
			header->set_arr(buf, len);
			header->set_arrsize(len);
			header->set_type(message->GetDescriptor()->full_name().data());

			delete[] buf;
			return header;
		}

		static __forceinline bool Pack(
			_In_ CKPacket::messageHeader* header,
			_In_ const char* serializedBuffer,
			_In_ size_t serializedBufferLength,
			_In_ const char* typeBuffer,
			_Out_ char* packedBuffer,
			_Out_ size_t* packedLength,
			_In_ size_t packedBufferMax
		)
		{
			header->set_arr(serializedBuffer, serializedBufferLength);
			header->set_arrsize(serializedBufferLength);
			header->set_type(typeBuffer);

			bool rtv = Serialize(header, packedBuffer + sizeof(__int32), packedBufferMax, packedLength);
			if (rtv == false) return false;

			Int32ToByteArr(*packedLength, packedBuffer);

			*packedLength = *packedLength  + sizeof(__int32);

			return true;
		}

		static __forceinline bool Unpack(
			_In_opt_ google::protobuf::Arena* arena,
			_In_ const char* packedBuffer,
			_In_ size_t packedBufferLength,
			_Out_ CKPacket::messageHeader** header
		)
		{
			static const char* type = CKPacket::messageHeader::GetDescriptor()->full_name().data();

			if (header == nullptr)return false;

			__int32 msgLen = ByteArrToInt32(packedBuffer);


			return Deserialize(arena, packedBuffer + sizeof(__int32), msgLen, type, header);
		}

		static __forceinline bool Serialize(
			_In_ PMessage msg,
			_Out_ char* buffer,
			_In_ size_t bufferMaxLength,
			_Out_ size_t* length
		)
		{
			if (msg == nullptr) return false;

			size_t size = msg->ByteSizeLong();
			if (bufferMaxLength < size) return false;

			msg->SerializeToArray(buffer, size);

			*length = size;

			return true;
		}

		template<class MESSAGE>
		static __forceinline bool Deserialize(
			_In_opt_ google::protobuf::Arena* arena,
			_In_ const char* const buffer,
			_In_ size_t length,
			_In_ const char* type,
			_Out_ MESSAGE** message
		)
		{
			*message = nullptr;

			auto desc = google::protobuf::DescriptorPool::generated_pool()->FindMessageTypeByName(type);

			if (desc == nullptr)
			{
				return false;
			}

			PMessage tempMessage = nullptr;
			
			tempMessage = google::protobuf::Arena::CreateMessage<MESSAGE>(arena);

			if (!tempMessage->ParseFromArray(buffer, length))
			{
				return false;
			}

			*message = dynamic_cast<MESSAGE*>(tempMessage);
			if (*message == nullptr)
			{
				return false;
			}

			return true;
		}
	};
}


#pragma warning(pop)
//---------------------------------------------------------------------------------------------------------------------
//  DVSAL
//---------------------------------------------------------------------------------------------------------------------
//  Copyright 2020 - Marco Montes Grova (a.k.a. mgrova) marrcogrova@gmail.com 
//---------------------------------------------------------------------------------------------------------------------
//  Permission is hereby granted, free of charge, to any person obtaining a copy of this software 
//  and associated documentation files (the "Software"), to deal in the Software without restriction, 
//  including without limitation the rights to use, copy, modify, merge, publish, distribute, 
//  sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is 
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in all copies or substantial 
//  portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING 
//  BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
//  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES 
//  OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN 
//  CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//---------------------------------------------------------------------------------------------------------------------

#include <dvsal/streamers/AEDAT4Streamer.h>

namespace dvsal{
	AEDAT4Streamer::AEDAT4Streamer(std::istream *_inStream, const InputInformation *_inInfo){
		inputStream_ = _inStream;
		inputInfo_   = _inInfo;
	}

	bool AEDAT4Streamer::init() {

		cacheBuffer_ = dv::FileBuffer(inputInfo_->dataTable_->Table);

		// Build LZ4 decompression context.
		if (inputInfo_->compression_ == dv::CompressionType::LZ4 || inputInfo_->compression_ == dv::CompressionType::LZ4_HIGH) {
			compressionLZ4_ = lz4InitDecompressionContext();
		}

		// Build Zstd decompression context.
		if (inputInfo_->compression_ == dv::CompressionType::ZSTD || inputInfo_->compression_ == dv::CompressionType::ZSTD_HIGH) {
			compressionZstd_ = zstdInitDecompressionContext();
		}

		int64_t startTimePos = 0;
		int64_t endTimePos   = 0;

		int64_t startTimestamp = startTimePos + inputInfo_->timeLowest_;
		int64_t endTimestamp   = endTimePos + inputInfo_->timeHighest_;

			// Now we know how much time we need to extract, from when to when.
			// First let's see if there is even something for that time interval for
			// each of the streams.
		
		cacheBuffer_.updatePacketsTimeRange(startTimestamp, endTimestamp, 0 );
		packetsToRead_ = cacheBuffer_.getInRange();

		if (packetsToRead_.empty()) {
			// No data for this stream and time interval, skip.
			std::cout << "Packets to read empty \n";
		}

		std::cout << "Packets to read: " << packetsToRead_.size() << std::endl;
		iterPackets_ = packetsToRead_.begin();

		return true;
	}

	InputInformation AEDAT4Streamer::parseHeader(std::ifstream &_fStream) {
		// Extract AEDAT version.
		char aedatVersion[static_cast<std::underlying_type_t<dv::Constants>>(dv::Constants::AEDAT_VERSION_LENGTH)];
		_fStream.read(
			aedatVersion, static_cast<std::underlying_type_t<dv::Constants>>(dv::Constants::AEDAT_VERSION_LENGTH));

		// Check if file is really AEDAT 4.0.
		if (memcmp(aedatVersion, "#!AER-DAT4.0\r\n",
				static_cast<std::underlying_type_t<dv::Constants>>(dv::Constants::AEDAT_VERSION_LENGTH))
			!= 0) {
			throw std::runtime_error("AEDAT4.0: no valid version line found.");
		}

		InputInformation result;

		// We want to understand what data is in this file,
		// and return actionable elements to our caller.
		// Parse version 4.0 header content.
		// Uses IOHeader flatbuffer. Get its size first.
		flatbuffers::uoffset_t ioHeaderSize;

		_fStream.read(reinterpret_cast<char *>(&ioHeaderSize), sizeof(ioHeaderSize));

		// Size is little-endian.
		ioHeaderSize = le32toh(ioHeaderSize);

		// Allocate memory for header and read it.
		auto ioHeader = std::make_unique<uint8_t[]>(ioHeaderSize);

		_fStream.read(reinterpret_cast<char *>(ioHeader.get()), ioHeaderSize);

		// Get file size (should always work in binary mode).
		_fStream.seekg(0, std::ios::end);
		size_t fileSize = static_cast<size_t>(_fStream.tellg());

		// Back to right after the header.
		_fStream.seekg(static_cast<std::underlying_type_t<dv::Constants>>(dv::Constants::AEDAT_VERSION_LENGTH)
						  + sizeof(ioHeaderSize) + ioHeaderSize,
			std::ios::beg);

		// Verify header content is valid.
		flatbuffers::Verifier ioHeaderVerify(ioHeader.get(), ioHeaderSize);

		if (!dv::VerifyIOHeaderBuffer(ioHeaderVerify)) {
			throw std::runtime_error("AEDAT4.0: could not verify IOHeader contents.");
		}

		// Parse header into C++ class.
		auto header = dv::UnPackIOHeader(ioHeader.get());

		// Set file-level return information.
		result.fileSize_          = fileSize;
		result.compression_       = header->compression;
		result.dataTablePosition_ = header->dataTablePosition;
		result.dataTableSize_
			= (header->dataTablePosition < 0) ? (0) : (fileSize - static_cast<size_t>(header->dataTablePosition));

		// Get file data table to determine seek position.
		result.dataTable_ = loadFileDataTable(_fStream, result);

		if (!result.dataTable_->Table.empty()) {
			int64_t lowestTimestamp  = INT64_MAX;
			int64_t highestTimestamp = 0;

			for (const auto &inputDef : result.dataTable_->Table) {
				// No timestamp is denoted by -1.
				if (inputDef.TimestampStart == -1) {
					continue;
				}

				// Determine lowest and highest timestamps present in file.
				if (inputDef.TimestampStart < lowestTimestamp) {
					lowestTimestamp = inputDef.TimestampStart;
				}

				if (inputDef.TimestampEnd > highestTimestamp) {
					highestTimestamp = inputDef.TimestampEnd;
				}
			}

			result.timeLowest_  = lowestTimestamp;
			result.timeHighest_ = highestTimestamp;
		}
		else {
			result.timeLowest_  = 0;
			result.timeHighest_ = INT64_MAX;
		}

		result.timeDifference_ = result.timeHighest_ - result.timeLowest_;
		result.timeShift_      = result.timeLowest_;

		return (result);
	}

	std::ifstream AEDAT4Streamer::openFile(const std::filesystem::path &fPath) {
		if (!std::filesystem::exists(fPath) || !std::filesystem::is_regular_file(fPath)) {
			throw std::runtime_error("File doesn't exist or cannot be accessed.");
		}

		if (fPath.extension().string() != (".aedat4")) {
			throw std::runtime_error("Unknown file extension '" + fPath.extension().string() + "'.");
		}

		std::ifstream fStream;

		// Enable exceptions on failure to open file.
		fStream.exceptions(std::ifstream::badbit | std::ifstream::failbit | std::ifstream::eofbit);

		fStream.open(fPath.string(), std::ios::in | std::ios::binary);

		return (fStream);
	}

    std::unique_ptr<dv::FileDataTable> AEDAT4Streamer::loadFileDataTable(std::ifstream &_fStream, InputInformation &_fInfo) {
        // Only proceed if data table is present.
        if (_fInfo.dataTablePosition_ < 0) {
            // Return empty table.
            return (std::unique_ptr<dv::FileDataTable>(new dv::FileDataTable{}));
        }

        // Remember offset to go back here after.
        auto initialOffset = _fStream.tellg();

        // Read file table data from file.
        std::vector<char> dataTableBuffer;
        dataTableBuffer.resize(_fInfo.dataTableSize_);

        _fStream.seekg(_fInfo.dataTablePosition_);
        _fStream.read(dataTableBuffer.data(), static_cast<int64_t>(_fInfo.dataTableSize_));

        // Decompress.
        size_t dataTableSize     = dataTableBuffer.size();
        const char *dataTablePtr = dataTableBuffer.data();

        std::vector<char> decompressBuffer;
        decompressBuffer.reserve(dataTableSize);

        if (_fInfo.compression_ != dv::CompressionType::NONE) {
            if (_fInfo.compression_ == dv::CompressionType::LZ4 || _fInfo.compression_ == dv::CompressionType::LZ4_HIGH) {
                auto ctx = lz4InitDecompressionContext();

                decompressLZ4(dataTablePtr, dataTableSize, decompressBuffer, ctx.get());
            }

            if (_fInfo.compression_ == dv::CompressionType::ZSTD || _fInfo.compression_ == dv::CompressionType::ZSTD_HIGH) {
                auto ctx = zstdInitDecompressionContext();

                decompressZstd(dataTablePtr, dataTableSize, decompressBuffer, ctx.get());
            }

            dataTablePtr  = decompressBuffer.data();
            dataTableSize = decompressBuffer.size();
        }

        // Verify header content is valid.
        flatbuffers::Verifier dataTableVerify(reinterpret_cast<const uint8_t *>(dataTablePtr), dataTableSize);

        if (!dv::VerifySizePrefixedFileDataTableBuffer(dataTableVerify)) {
            throw std::runtime_error("AEDAT4.0: could not verify IOHeader contents.");
        }

        // Unpack table.
        auto dataTableFlatbuffer = dv::GetSizePrefixedFileDataTable(dataTablePtr);
        auto dataTable           = std::unique_ptr<dv::FileDataTable>(dataTableFlatbuffer->UnPack());

        // Reset file position to initial one.
        _fStream.seekg(initialOffset);

        return (dataTable);
    }
    
    
    std::shared_ptr<struct LZ4F_dctx_s> AEDAT4Streamer::lz4InitDecompressionContext() {
		// Create LZ4 decompression context.
		struct LZ4F_dctx_s *ctx = nullptr;
		auto ret                = LZ4F_createDecompressionContext(&ctx, LZ4F_VERSION);
		if (ret != 0) {
			throw std::bad_alloc();
		}

		return (std::shared_ptr<struct LZ4F_dctx_s>(ctx, [](struct LZ4F_dctx_s *c) {
			LZ4F_freeDecompressionContext(c);
		}));
	}


    void AEDAT4Streamer::decompressLZ4(const char *_dataPtr, size_t _dataSize, std::vector<char> &_decompressBuffer,
		struct LZ4F_dctx_s *_decompressContext) {
		size_t decompressedSize = 0;
		size_t retVal           = 1;

		while ((_dataSize > 0) && (retVal != 0)) {
			_decompressBuffer.resize(decompressedSize + lz4CompressionChunkSize_);
			size_t dstSize = lz4CompressionChunkSize_;

			size_t srcSize = _dataSize;

			retVal = LZ4F_decompress(
				_decompressContext, _decompressBuffer.data() + decompressedSize, &dstSize, _dataPtr, &srcSize, nullptr);

			if (LZ4F_isError(retVal)) {
				throw std::runtime_error(std::string("LZ4 decompression error: ") + LZ4F_getErrorName(retVal));
			}

			decompressedSize += dstSize;

			_dataSize -= srcSize;
			_dataPtr += srcSize;
		}

		_decompressBuffer.resize(decompressedSize);
	}


	std::shared_ptr<struct ZSTD_DCtx_s> AEDAT4Streamer::zstdInitDecompressionContext() {
		// Create Zstd decompression context.
		struct ZSTD_DCtx_s *ctx = ZSTD_createDCtx();
		if (ctx == nullptr) {
			throw std::bad_alloc();
		}

		return (std::shared_ptr<struct ZSTD_DCtx_s>(ctx, [](struct ZSTD_DCtx_s *c) {
			ZSTD_freeDCtx(c);
		}));
	}

    void AEDAT4Streamer::decompressZstd(const char *_dataPtr, size_t _dataSize, std::vector<char> &_decompressBuffer,
		struct ZSTD_DCtx_s *_decompressContext) {
		auto decompressedSize = ZSTD_getFrameContentSize(_dataPtr, _dataSize);
		if (decompressedSize == ZSTD_CONTENTSIZE_UNKNOWN) {
			throw std::runtime_error("Zstd decompression error: unknown content size");
		}
		if (decompressedSize == ZSTD_CONTENTSIZE_ERROR) {
			throw std::runtime_error("Zstd decompression error: content size error");
		}

		_decompressBuffer.resize(decompressedSize);

		auto retVal
			= ZSTD_decompressDCtx(_decompressContext, _decompressBuffer.data(), decompressedSize, _dataPtr, _dataSize);

		if (ZSTD_isError(retVal)) {
			throw std::runtime_error(std::string("Zstd decompression error: ") + ZSTD_getErrorName(retVal));
		}
		else {
			_decompressBuffer.resize(retVal);
		}
	}

	void AEDAT4Streamer::decompressData(const char *_dataPtr, size_t _dataSize) {
		if (inputInfo_->compression_ == dv::CompressionType::LZ4 || inputInfo_->compression_ == dv::CompressionType::LZ4_HIGH) {
			try {
				decompressLZ4(_dataPtr, _dataSize, decompressBuffer_, compressionLZ4_.get());
			}
			catch (const std::runtime_error &ex) {
                // Decompression error, ignore this packet.
                #if defined(LZ4_VERSION_NUMBER) && LZ4_VERSION_NUMBER >= 10800
                    LZ4F_resetDecompressionContext(compressionLZ4.context.get());
                #else
                    compressionLZ4_.reset();
                    compressionLZ4_ = lz4InitDecompressionContext();
                #endif
                
                std::cout << ex.what() << std::endl;
                return;
			}
		}

		if (inputInfo_->compression_ == dv::CompressionType::ZSTD || inputInfo_->compression_ == dv::CompressionType::ZSTD_HIGH) {
			try {
				decompressZstd(_dataPtr, _dataSize, decompressBuffer_, compressionZstd_.get());
			}
			catch (const std::runtime_error &ex) {
				// Decompression error, ignore this packet.
                #if defined(ZSTD_VERSION_NUMBER) && ZSTD_VERSION_NUMBER >= 10400
				    ZSTD_DCtx_reset(compressionZstd.context.get(), ZSTD_reset_session_only);
                #else
				    compressionZstd_.reset();
				    compressionZstd_ = zstdInitDecompressionContext();
                #endif
                
				std::cout << ex.what() << std::endl;
				return;
			}
		}
	}

	bool AEDAT4Streamer::step(){
		
		const char *dataPtr;
		std::size_t dataSize;
		if (iterPackets_->cached) {
			dataPtr  = cacheBuffer_.getDataPtrCache(*iterPackets_).data();
			dataSize = cacheBuffer_.getDataSizeCache(*iterPackets_);
		}
		else {
			inputStream_->seekg(iterPackets_->packet.ByteOffset, std::ios_base::beg);

			readBuffer_.resize(static_cast<size_t>(iterPackets_->packet.PacketInfo.Size()));

			inputStream_->read(readBuffer_.data(), iterPackets_->packet.PacketInfo.Size());

			dataSize = readBuffer_.size();
			dataPtr  = readBuffer_.data();

			// Decompress packet.
			if (inputInfo_->compression_ != dv::CompressionType::NONE) {
				decompressData(dataPtr, dataSize);

				dataSize = decompressBuffer_.size();
				dataPtr  = decompressBuffer_.data();
			}
			// add dataPtr and dataSize to cache
			std::vector<char> temp(dataPtr, dataPtr + dataSize);
			cacheBuffer_.addToCache(*iterPackets_, temp, dataSize);
		}

		// dataPtr and dataSize now contain the uncompressed, raw flatbuffer.
		if (!flatbuffers::BufferHasIdentifier(dataPtr, "EVTS", true)) {
			// Wrong type identifier for this flatbuffer (file_identifier field).
			// This should never happen, ignore packet.
			std::cout << " Flatbuffer identifier is 'EVTS' \n";
			return false;
		}

		// Unpack event packet mode 1.
		auto dataEventPacket = dv::GetSizePrefixedEventPacket(dataPtr);
		auto eventPacket     = std::unique_ptr<dv::EventPacket>(dataEventPacket->UnPack());

		// Unpack event packet mode 2.
		// auto fbPtr          = flatbuffers::GetSizePrefixedRoot<void>(dataPtr);
		// auto eventsPacketFb = reinterpret_cast<const dv::EventPacketFlatbuffer*>(fbPtr);
		// auto e = ev->UnPack();	

		for(auto ev: eventPacket->elements)
			lastEvents_.add(ev);

		bool lastPacket = (std::next(iterPackets_) == packetsToRead_.end());

		if (lastPacket || iterPackets_ == packetsToRead_.end() ){
			std::cout << "Finished file \n";
			return false;
		}else{
			iterPackets_++;
			return true;
		}
	}

	void AEDAT4Streamer::events(dv::EventStore &_events , int _microseconds){
		
		return;
	}

    bool AEDAT4Streamer::image(cv::Mat &_image){
		return false;
	}

}
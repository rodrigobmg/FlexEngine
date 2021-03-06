#pragma once

#include <string>
#include <vector>
#include <direct.h> // For _getcwd

#pragma warning(push, 0)
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <assimp/vector2.h>
#include <assimp/vector3.h>
#include <assimp/color4.h>

#include "imgui.h"
#pragma warning(pop)

#include "Graphics/RendererTypes.hpp"
#include "Types.hpp"

namespace flex
{
	extern ImVec4 g_WarningTextColor;
	extern ImVec4 g_WarningButtonColor;
	extern ImVec4 g_WarningButtonHoveredColor;
	extern ImVec4 g_WarningButtonActiveColor;

	GLFWimage LoadGLFWimage(const std::string& filePath, i32 requestedChannelCount = 3, bool flipVertically = false, i32* channelCountOut = nullptr);
	void DestroyGLFWimage(GLFWimage& image);

	struct HDRImage
	{
		bool Load(const std::string& hdrFilePath, i32 requestedChannelCount, bool flipVertically);
		void Free();

		i32 width;
		i32 height;
		i32 channelCount;
		std::string filePath;
		real* pixels;
	};

	template <class T>
	struct RollingAverage
	{
		RollingAverage();
		RollingAverage(i32 valueCount);

		void AddValue(T newValue);
		void Reset();

		T currentAverage;
		std::vector<T> prevValues;

		i32 currentIndex = 0;
	};

	template<class T>
	inline RollingAverage<T>::RollingAverage()
	{
	}

	template <class T>
	RollingAverage<T>::RollingAverage(i32 valueCount)
	{
		prevValues.resize(valueCount);
	}

	template <class T>
	void RollingAverage<T>::AddValue(T newValue)
	{
		prevValues[currentIndex++] = newValue;
		currentIndex %= prevValues.size();

		currentAverage = T();
		std::for_each(prevValues.begin(), prevValues.end(), [&](T value)
		{
			currentAverage += value;
		});

		currentAverage /= prevValues.size();
	}

	template <class T>
	void RollingAverage<T>::Reset()
	{
		for (T& v : prevValues)
		{
			v = T();
		}
		currentIndex = 0;
		currentAverage = T();
	}
	
	// Stores text render commands issued during the 
	// frame to later be converted to "TextVertex"s
	struct TextCache
	{
	public:
		TextCache(const std::string& text, AnchorPoint anchor, glm::vec2 position, glm::vec4 col, real xSpacing, bool bRaw);

		std::string str;
		AnchorPoint anchor;
		glm::vec2 pos;
		glm::vec4 color;
		real xSpacing;
		std::vector<real> letterYOffsets;
		bool bRaw;

	private:
		//TextCache& operator=(const TextCache &tmp);

	};

	bool FileExists(const std::string& filePath);

	bool ReadFile(const std::string& filePath, std::string& fileContents, bool bBinaryFile);
	bool ReadFile(const std::string& filePath, std::vector<char>& vec, bool bBinaryFile);

	bool WriteFile(const std::string& filePath, const std::string& fileContents, bool bBinaryFile);
	bool WriteFile(const std::string& filePath, const std::vector<char>& vec, bool bBinaryFile);

	bool DeleteFile(const std::string& filePath, bool bPrintErrorOnFailure = true);

	bool CopyFile(const std::string& filePathFrom, const std::string& filePathTo);

	bool DirectoryExists(const std::string& absoluteDirectoryPath);

	// Returns true if any files were found
	// Set fileType to "*" to retrieve all files
	bool FindFilesInDirectory(const std::string& directoryPath, std::vector<std::string>& filePaths, const std::string& fileType);

	// Removes all content before final '/' or '\' 
	void StripLeadingDirectories(std::string& filePath);

	// Removes all content after final '/' or '\'
	// NOTE: If path describes a directory and doesn't end in a slash, final directory will be removed
	void ExtractDirectoryString(std::string& filePath);

	// Removes all chars after first '.' occurrence
	void StripFileType(std::string& filePath);

	// Removes all chars before first '.' occurrence
	void ExtractFileType(std::string& filePathInTypeOut);

	// Creates directories for each listed in string
	void CreateDirectoryRecursive(const std::string& absoluteDirectoryPath);

	/*
	* Reads in a .wav file and fills in given values according to file contents
	* Returns true if reading and parsing succeeded
	*/
	bool ParseWAVFile(const std::string& filePath, i32* format, u8** data, i32* size, i32* freq);

	/* Interpret 4 bytes starting at ptr as an unsigned 32-bit int */
	u32 Parse32u(char* ptr);
	/* Interpret 2 bytes starting at ptr as an unsigned 16-bit int */
	u16 Parse16u(char* ptr);


	std::vector<std::string> Split(const std::string& str, char delim);

	/*
	 * Returns the index of the first character which isn't a number
	 * of letter (or -1 if none exist) starting from offset
	 */
	i32 NextNonAlphaNumeric(const std::string& str, i32 offset);

	real Lerp(real a, real b, real t);
	glm::vec2 Lerp(const glm::vec2& a, const glm::vec2& b, real t);
	glm::vec3 Lerp(const glm::vec3& a, const glm::vec3& b, real t);

	/* Parses a single float, returns -1.0f if incorrectly formatted */
	real ParseFloat(const std::string& floatStr);

	/* Parses a comma separated list of 2 floats */
	glm::vec2 ParseVec2(const std::string& vecStr);

	/* Parses a comma separated list of 3 floats */
	glm::vec3 ParseVec3(const std::string& vecStr);

	/*
	* Parses a comma separated list of 3 or 4 floats
	* If defaultW is non-negative and no w component exists in the string defaultW will be used
	* If defaultW is negative then an error will be generated if the string doesn't
	* contain 4 components
	*/
	glm::vec4 ParseVec4(const std::string& vecStr, real defaultW = 1.0f);

	bool IsNanOrInf(const glm::vec2& vec);
	bool IsNanOrInf(const glm::vec3& vec);
	bool IsNanOrInf(const glm::vec4& vec);
	bool IsNanOrInf(const glm::quat& quat);

	template<class T>
	inline typename std::vector<T>::const_iterator Contains(const std::vector<T>& vec, const T& t)
	{
		for (std::vector<T>::const_iterator iter = vec.begin(); iter != vec.end(); ++iter)
		{
			if (*iter == t)
			{
				return iter;
			}
		}

		return vec.end();
	}

	template<class TReal>
	glm::vec2 ToVec2(const aiVector2t<TReal>& vec)
	{
		return glm::vec2(vec.x, vec.y);
	}

	template<class TReal>
	glm::vec3 ToVec3(const aiVector3t<TReal>& vec)
	{
		return glm::vec3(vec.x, vec.y, vec.z);
	}

	template<class TReal>
	glm::vec4 ToVec4(const aiColor4t<TReal>& color)
	{
		return glm::vec4(color.r, color.g, color.b, color.a);
	}

	std::string FloatToString(real f, i32 precision);

	// String will be padded with '0's to be at least minChars long (excluding a leading '-' for negative numbers)
	std::string IntToString(i32 i, u16 minChars = 0);

	std::string Vec2ToString(const glm::vec2& vec);
	std::string Vec3ToString(const glm::vec3& vec);
	std::string Vec4ToString(const glm::vec4& vec);

	void CopyColorToClipboard(const glm::vec4& col);
	void CopyColorToClipboard(const glm::vec3& col);

	glm::vec3 PasteColor3FromClipboard();
	glm::vec4 PasteColor4FromClipboard();

	CullFace StringToCullFace(const std::string& str);
	std::string CullFaceToString(CullFace cullFace);

	void ToLower(std::string& str);
	void ToUpper(std::string& str);

	bool StartsWith(const std::string& str, const std::string& start);
	bool EndsWith(const std::string& str, const std::string& end);

	// Returns the number str ends with or -1 if last char isn't numeral
	// outNumNumericalChars will be set to the number of chars in the num (e.g. "001" => 3)
	i32 GetNumberEndingWith(const std::string& str, i16& outNumNumericalChars);

	const char* GameObjectTypeToString(GameObjectType type);
	GameObjectType StringToGameObjectType(const char* gameObjectTypeStr);

	// Must be called at least once to set g_CurrentWorkingDirectory!
	void RetrieveCurrentWorkingDirectory();
	std::string RelativePathToAbsolute(const std::string& relativePath);
} // namespace flex

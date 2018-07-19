#pragma once

#include <string>



struct VertexAttribute
{
public:
	VertexAttribute() {}
	VertexAttribute( const std::string& name, unsigned int type, unsigned int elementCount, bool normalize, size_t offset );
	// Default copy

	bool operator==(const VertexAttribute& rhs) const;
	bool operator!=(const VertexAttribute& rhs) const;

	const std::string&	GetName() const;
	unsigned int		GetGLType() const;
	int					GetElementCount() const;
	int					GetMemberOffset() const;
	bool				ShouldNormalize() const;

	static const VertexAttribute NULL_ATTRIBUTE;


private:
	std::string	 m_name;		// POSITION, COLOR, UV

	unsigned int m_glType;
	int			 m_elementCount; 
	bool		 m_isNormalized; 

	int			 m_memberOffset; // how far from start of element to this piece of data
};
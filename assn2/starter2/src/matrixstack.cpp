#include "matrixstack.h"

MatrixStack::MatrixStack()
{
	m_matrices.push_back(Matrix4f::identity());
}

void MatrixStack::clear()
{
	m_matrices.clear();
	m_matrices.push_back(Matrix4f::identity());
}

Matrix4f MatrixStack::top()
{
	return m_matrices.back();
}

void MatrixStack::push(const Matrix4f& m)
{
	m_matrices.push_back(top() * m);
	// Push m onto the stack.
	// The new top should be "old * m", so that conceptually the new matrix
	// is applied first in right-to-left evaluation.
}

void MatrixStack::pop()
{	
	// Remove the top element from the stack
	m_matrices.pop_back();
	if (m_matrices.size() <= 0)
	{
		m_matrices.push_back(Matrix4f::identity());
	}
}

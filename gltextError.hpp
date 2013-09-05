#ifndef GLTEXTERROR_HPP
#define GLTEXTERROR_HPP

namespace gltext
{

#define GLTEXT_ERROR_STRING(X) #X

/**
 * Runtime error in case we don`t have exception support
 */
class Error
{
public:

	/**
	 * Throw an error
	 */
	template <typename T>
	static void throwError()
	{
#ifndef GLTEXT_NO_EXCEPTIONS
		throw T();
#else
		assert(false && "Exception: " GLTEXT_ERROR_STRING(T));
#endif
	}
};

}

#endif // GLTEXTERROR_HPP

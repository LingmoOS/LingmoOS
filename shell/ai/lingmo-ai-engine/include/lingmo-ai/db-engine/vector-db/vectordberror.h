#ifndef VECTORDBERROR_H
#define VECTORDBERROR_H
namespace db_engine {
namespace vector_db {
enum class VectorDBErrorCode
{
    Success,
    ArgumentError,
    PyObjectCallMethodError,
    ValidDataBase
};

}
}

#endif
#include <iostream>
#include <rapidjson/filereadstream.h>
#include <rapidjson/filewritestream.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/document.h>

using namespace rapidjson;
using namespace std;

template <typename Writer>
void display(Writer &writer);

template <typename Document>
void changeDom(Document &d);

int main()
{
    const string CONFIG_FILE_PATH = "config/example.json";

    StringBuffer s;
    Document d;
    PrettyWriter<StringBuffer> writer(s);

    FILE *fp = fopen(CONFIG_FILE_PATH.c_str(), "rb");

    char readBuffer[65536];
    FileReadStream is(fp, readBuffer, sizeof(readBuffer));

    d.ParseStream(is);

    fclose(fp);

    changeDom(d);

    s.Clear();
    writer.Reset(s);
    d.Accept(writer);

    cout << s.GetString() << endl;
}

template <typename Writer>
void display(Writer &writer)
{
    writer.StartObject();

    writer.String("hello");
    writer.String("world");

    writer.String("t");
    writer.Bool(true);

    writer.String("f");
    writer.Bool(false);

    writer.String("n");
    writer.Null();

    writer.String("i");
    writer.Uint(123);

    writer.String("pi");
    writer.Double(3.1416);

    writer.String("a");
    writer.StartArray();
    for (int i = 0; i < 4; i++)
    {
        writer.Uint(i);
    }
    writer.EndArray();

    writer.String("o");
    writer.StartObject();
    writer.String("prop1");
    writer.String("value1");
    writer.String("prop2");
    writer.String("value2");
    writer.EndObject();

    writer.EndObject();
}

template <typename Document>
void changeDom(Document &d)
{
    if (d.HasMember("hello")) {
        Value &node = d["hello"];
        node.SetString("c++");
    } else {
        d.AddMember("hello", "c++", d.GetAllocator());
    }

    Document subdoc(&d.GetAllocator());
    subdoc.SetObject();

    Value arr(kArrayType);
    Value::AllocatorType allocator;

    for (int i = 0; i < 10; i++)
    {
        arr.PushBack(i, allocator);
    }

    subdoc.AddMember("New", Value(kObjectType).Move().AddMember("Numbers", arr, allocator), subdoc.GetAllocator());

    d.AddMember("testing", subdoc, d.GetAllocator());
}
proto_path=../Proto
out_path=${proto_path}/src
build_path=../Common/proto
protoc=${proto_path}/protoc
files=$(ls ${proto_path}/*.proto)

rm -f src/*.h
rm -f src/*.cc
mkdir -p ${out_path}

for file in ${files}; do
    name=$(echo ${file} | awk -F"/" '{print $3}')
    echo $name
    ${protoc} -I${proto_path} --cpp_out=${out_path} ${name}
done

rm -rf ${build_path}/include/*.h
rm -rf ${build_path}/src/*.cc

cp -rf ${out_path}/*.pb.h ${build_path}/include/
cp -rf ${out_path}/*.pb.cc ${build_path}/src/


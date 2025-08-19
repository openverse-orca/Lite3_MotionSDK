## 构建OrcaGym适配版本的Lite3控制程序

1. 首先退出conda环境

```bash
conda deactivate
```

2. 运行构建脚本

```bash
./build.sh
```

3. 运行测试脚本

```bash
./test/run_grpc_test.sh
```

4. 启动推理服务端，在另一个终端中运行

```bash
conda activate orca
cd ${OrcaGym_path}/example/legged_gym/
python scripts/grpc_server.py --config configs/lite3_sim_config.yaml
```

5. 运行控制程序 

```bash
./run.sh
```
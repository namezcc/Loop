<template>
  <div>
    <el-scrollbar height="400px">
      <el-row>
        <span> {{name}} id:{{id}}</span>
      </el-row>
      <el-row>
        链接
      </el-row>
      <el-row>
        <el-scrollbar max-height="300px">
          <p v-for="l in serlink.Link" :key="l.Id" :class="[l.Open==0 ? 'errorbg':'openbg']">
            {{l.Name}} : {{l.Id}} port: {{l.Port}}
          </p>
        </el-scrollbar>
      </el-row>
    </el-scrollbar>
  </div>
</template>

<script>
export default {
  name: "serverInfo",
  props: {
    name: String,
    id: Number,
    serlink: Object,
  }
};
</script>

<style scoped>
.errorbg {
  color: #F56C6C;
}

.openbg {
  color: #67C23A;
}
</style>

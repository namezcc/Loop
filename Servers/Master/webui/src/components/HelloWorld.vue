<template>
  <div class="hello">
    <el-menu>
      <el-sub-menu>
        <template #title>
          <span>machine</span>
        </template>
        <el-menu-item v-for="s in msg" :key="s.id">
          <a @click="getmachine(s.id)">{{ s.id }}</a>
        </el-menu-item>
      </el-sub-menu>
    </el-menu>
  </div>
</template>

<script>
export default {
  name: "HelloWorld",
  props: {
    msg: Array,
    getmachine: Function,
  }
};
</script>

<!-- Add "scoped" attribute to limit CSS to this component only -->
<style scoped>
.hello {
  width: 200px;
  height: 100%;
}

h3 {
  margin: 40px 0 0;
}
ul {
  list-style-type: none;
  padding: 0;
}

a {
  display: block;
}
</style>

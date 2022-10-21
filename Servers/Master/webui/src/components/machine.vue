<template>
  <div class="machine">
    <el-scrollbar height="400px">
      <el-row>
        <el-col :span="12">
          <div class="openbg machine-title" v-if="msg.open == 1">
            {{ id }} 已开启 sernum {{ msg.server.length }}
          </div>
          <div class="closebg machine-title" v-else>
            {{ id }} 未开启
          </div>
        </el-col>
        <el-col :span="12">
          <el-button type= "primary" @click="openAllServer()">
            全部开启
          </el-button>
          <el-button type= "primary" @click="closeAllServer()">
            全部关闭
          </el-button>
        </el-col>
      </el-row>

      <template v-if="msg.server">
        <el-row v-for="s in msg.server" :key="s.type*10000 + s.id">
          <el-col :span="12">
            <div class="server-box" :class="{'closebg':s.open==0}">
              <span class="glyphicon glyphicon-ok-sign icircle" v-if="s.open==1" />
              <span>{{ s.name }} id: {{ s.id }} </span>
              <template v-if="s.open==1">
                <span>   link:</span>
                <span class="glyphicon glyphicon-ok-sign" :class="[ (s.error & 1)==0 ? 'icircle': 'errorbg'] " />
              </template>
              <template v-if="s.open==1 && (s.error & 2)>0">
                <span>   redis:</span>
                <span class="glyphicon glyphicon-ok-sign errorbg" />
              </template>
              <template v-if="s.open==1 && (s.error & 4)>0">
                <span>   mysql:</span>
                <span class="glyphicon glyphicon-ok-sign errorbg" />
              </template>
            </div>
          </el-col>
          <el-col :span="12">
            <el-button type = "primary" @click="getServer(s.name,s.type,s.id)">
              查看
            </el-button>
            <el-button type= "primary" icon="el-icon-switch-button" @click="serverOpt(s.type,s.id,s.open==0 ? 1 : 0)">
              
            </el-button>
          </el-col>
        </el-row>
      </template>

    </el-scrollbar>
  </div>
</template>

<script>
import constval from './constval.vue'
import axios from 'axios'

export default {
  name: "machine",
  data() {
    return {
      openbg: 'openbg',
      closebg: 'closebg',
    }
  },
  methods:{
    openAllServer(){
      var url = `serverOpt?mid=${this.id}&id=0&type=0&opt=1`
      axios.get(constval.server + url)
      .then((
        alert("已执行,刷新")
      ))
    },
    closeAllServer(){
      var url = `serverOpt?mid=${this.id}&id=0&type=0&opt=0`
      axios.get(constval.server + url)
      .then((
        alert("已执行,刷新")
      ))
    },
    serverOpt(sertype,serid,opt){
      var url = `serverOpt?mid=${this.id}&id=${serid}&type=${sertype}&opt=${opt}`
      axios.get(constval.server + url)
      .then((
        alert("已执行,刷新")
      ))
    },
  },
  props: {
    id: Number,
    msg: {
      type: Object,
      default(){
        return {open:false,server:[]}
      }
    },
    getServer: Function,
  }
};
</script>

<style scoped>
.machine {
  width: 600px;
}

.machine-title {
  height: 30px;
}

.el-col {
  line-height: 100%;
  text-align: center;
}

.server-box {
  height: 50px;
  margin-bottom: 2px;
  text-align: left;
  line-height: 50px;
  padding-left: 10px;
}

.icircle {
  color: #67C23A;
  margin-right: 5px;
}

.errorbg {
  color: #F56C6C;
}

.openbg {
  background-color: #67C23A;
}

.closebg {
  background-color: #909399;
}

</style>

<template>
  <HelloWorld :msg= info :getmachine= getMachineInfo />
  <machine :msg= minfo :id= machineId :getServer= getServerInfo />
  <serverInfo :name= sername :id=serid :serlink= serlink />
</template>

<script>
import HelloWorld from "./components/HelloWorld.vue";
import machine from "./components/machine.vue"
import serverInfo from "./components/serverInfo.vue"
import constval from './components/constval.vue'
import axios from 'axios';

export default {
  name: "App",
  components: {
    HelloWorld,
    machine,
    serverInfo,
  },
  data() {
    return {
      info: null,
      minfo: {open:false,server:[]},
      machineId: 0,
      sername: "",
      serid: 0,
      serlink:{Link:[]},
    };
  },
  mounted() {
    axios.get(constval.server + "machine")
    .then(Response => (this.info = Response.data))
    .catch(function (err) {
      console.log(err)
    })
  },
  methods: {
    getMachineInfo(id){
      this.machineId = id
      axios.get(constval.server + "viewMachine?id="+id)
      .then(Response => (this.minfo = Response.data))
      .catch(function (err) {
        console.log(err)
      })
    },
    getServerInfo(name,type,id){
      this.sername = name
      this.serid=id
      var url = `getServer?mid=${this.machineId}&id=${id}&type=${type}`
      
      axios.get(constval.server+url)
      .then(Response => (this.serlink = Response.data))
      .catch(function (err) {
        console.log(err)
      })
    }
  }
};
</script>

<style>
#app {
  font-family: Avenir, Helvetica, Arial, sans-serif;
  -webkit-font-smoothing: antialiased;
  -moz-osx-font-smoothing: grayscale;
  text-align: center;
  color: #2c3e50;
  margin-top: 60px;
  height: 80%;
  padding: 0px;
  display: flex;
}

html,body{
  height: 100%;
  margin: 0px;
  padding: 0px;
}

</style>

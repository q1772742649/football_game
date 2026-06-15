import { ZqzbComponent } from './zqzbComponent.js'
import { ZqbsComponent } from './zqbsComponent.js'
const app =  Vue.createApp({
    components: {
        ZqzbComponent,
        ZqbsComponent
    },
    data() {
        return {
            notShowJcJfbAry:[], //竞彩开售不展示积分榜的赛事，填入相应的赛事的ID，如多个请用,分割 如['1234','3456']
            notShowWbsjJfbAry:[], // 竞彩未开售的赛事列表
            notShowJcShooterAry:[], //竞彩开售不展示射手信息模块，如多个请用,分割 如['1234','3456']
            notShowWbsjShooterAry:[], // 不竞彩未开售的赛事列表
            notShowJcInjuryAry:[], //竞彩开售不展示伤停的赛事，如多个请用,分割 如['1234','3456']
            notShowWbsjInjuryAry:[], // 竞彩未开售的赛事列表
            isShowJfb:true, //false 为不展示积分榜
            isShowShooter:true, //false 为不展示射手榜
            isShowInjury:true, //false 为不展示伤停
            showType: 1, // 1 比赛信息, 2 比赛数据 ,3 固定奖金 ,4 比赛直播
            mid: this.getQueryKey('mid') , //竞彩id
            gm: this.getQueryKey('gm'),   //比赛id
            midStr:"",
            httpURl: jsCommonDataV1.webApi, //调用域名
            oddsHistory: [],
            contList:[],
            relatedNews:[],
            sectionsNo999:'',
            loadShow:true,
            FBonus:{matchResultByKey: {'had':"",'hhad':"",'crs':"",'ttg':"",'hafu':""},
                listSh:['s01s00','s02s00','s02s01','s03s00','s03s01','s03s02','s04s00','s04s01','s04s02','s05s00','s05s01','s05s02','s-1sh'],
                listSd:['s00s00','s01s01','s02s02','s03s03','s-1sd'],
                listSa:['s00s01','s00s02','s01s02','s00s03','s01s03','s02s03','s00s04','s01s04','s02s04','s00s05','s01s05','s02s05','s-1sa'],
                listHafu:['hh','hd','ha','dh','dd','da','ah','ad','aa'],
                listTtg:['s0','s1','s2','s3','s4','s5','s6','s7'],
                 hhadRn:{'h':'让胜','d':'让平','a':'让负'}
            },
            matchFeature:{},
            featureSort1 :['last','sameHomeAway','eachHomeAway','eachSameHomeAway'],
            limit:5,
            tournamentFlag:0,
            homeAwayFlag:0,
            historyOk:false,
            pageTitle:{},
            pageType:0
        }
    },
    created() {
        this.getFBTitle();
        this.showType = this.getQueryKey('showType') || 1
        if(this.mid == ''){
            this.midStr = 'matchId='+this.gm
            if(this.showType > 2){
                this.showType = 1
            }
        }else{
            this.midStr = 'sportteryMatchId='+this.mid
        }
        this.callData()
    },
    mounted(){
    },
    destroyed () {},

    methods: {
        setTrackEvent(eventName, data={}){
            try{
                dc.trackEvent(eventName, {'desc':JSON.stringify(data)})
            }catch(error){

            }
        },
        changeLoadShow(d){
            this.loadShow =d
        },
        async getFBTitle(){
            try{
                let sendUrl = this.httpURl+'/gateway/uniform/football/getMatchHeadV1.qry?source=web';
                if(this.mid != null && this.mid !=''){
                    sendUrl += '&sportteryMatchId='+this.mid
                }else if (this.gm != null && this.gm !='') {
                    sendUrl += '&matchId='+this.gm
                } else {
                    return
                }
                let res = await axios.get(sendUrl);
                if ( res.status == 200 && res.data.errorCode == "0" && JSON.stringify(res.data.value) !== '{}') {
                    this.pageTitle = res.data.value
                }
            } catch (err) {
                console.log(err);
            }
            this.pageType = 1
            //用于处理应急处理，前端展示积分有错误的情况
            if(!this.isShowJfb){
                this.pageTitle.wbsjStats = {}
            }
        },
        // tab 更换
        tabClick(id){
            if(parseInt(this.showType) === id){
                return
            }
            this.showType = id;
            this.callData()
        },
        getYMDStr(value){
            if(value==="") return ""
            return value.split(' ')[0]
        },
        getMDStr(value){
            if(value==="") return ''
            return value.split(' ')[0].substring(5)
        },
        // 获取URL参数
        getQueryKey(kName) {
            const query = new URLSearchParams(window.location.search);
            if(query.get(kName)== null){
                return ''
            }else{
                return query.get(kName)
            }

        },
        // 调用接口获取数据
        callData(){
            this.loadShow = true
            this.getJfbShow()
            switch (parseInt(this.showType)) {
                case 1:
                    this.setTrackEvent('click_matchData_sheetChange',{'dataType':'matchInfo'});
                    this.getDataBsxx();
                    break;
                case 2:
                    this.setTrackEvent('click_matchData_sheetChange',{'dataType':'matchDetail'});
                    this.getbsCheck();
                    break;
                case 3:
                    this.setTrackEvent('click_matchData_sheetChange',{'dataType':'fixedBonus'});
                    this.getFixedBonus();
                    break;
                case 4:
                    this.setTrackEvent('click_matchData_sheetChange',{'dataType':'matchLive'});
                    this.getzbCheck();
                    break;
                default:
                    break;
            }
        },
        getzbCheck(){

            if(this.mid ==""){
                this.loadShow = false
            }
        },
        getbsCheck(){
            if(this.mid =="" && this.gm ==""){
                this.loadShow = false
            }
        },
        // 显示比例条颜色
        ratioColor(a,b){
            if(a===0 && b===0){
                return''
            }else if(a>b){
                return 'u-bgRed';
            }else if(a === b){
                return 'u-bgGrey';
            }else if(a<b){
                return 'u-bgGrey';
            }else{
                return ''
            }
        },
        // 固定奖金
        async getFixedBonus(){
            try {
                let sendUrl = this.httpURl+'/gateway/uniform/football/getFixedBonusV1.qry?clientCode='
                    + commonV1Fun.comClientCode + '&matchId=' + this.mid;
                if(this.mid == null || this.mid == ""){
                    this.loadShow = false
                    return
                }
                this.FBonus.sectionsNo999 = '-2';
                this.FBonus.isCancel =0;
                let res = await  axios.get(sendUrl);
                if ( res.data.errorCode == 0 && Object.keys(res.data.value).length > 0) {
                        this.FBonus.matchResultList = res.data.value.matchResultList || [];
                        this.FBonus.oddsHistory = res.data.value.oddsHistory || [];
                        this.FBonus.sectionsNo999 = res.data.value.sectionsNo999;
                        this.FBonus.isCancel = res.data.value.isCancel
                        const that = this
                        this.FBonus.matchResultList.forEach(function(h){
                            if(that.FBonus.matchResultByKey.hasOwnProperty(h.code.toLowerCase())){
                                that.FBonus.matchResultByKey[h.code.toLowerCase()] = h
                            }

                        })
                   // }
                }
                this.loadShow = false
            } catch (err) {
                this.loadShow = false
                console.log(err);
            }
        },
        // 比赛信息
        async getDataBsxx() {
            try {
                let sendUrl = this.httpURl+'/gateway/info/getInfoListByMatchV2.inf?siteId=6'
                +'&sportType=1&isSearchRelated=1&termLimits=6&channelId=20002,20003';
                if(this.mid != null && this.mid !=''){
                    sendUrl += '&sportteryMatchId='+this.mid
                }else if (this.gm != null && this.gm !='') {
                    sendUrl += '&matchId='+this.gm
                } else {
                    this.loadShow = false
                    return
                }
                let res = await  axios.get(sendUrl);
                if ( res.status == 200 && res.data.errorCode == "0" && JSON.stringify(res.data.value) !== '{}') {
                    this.contList = res.data.value.contList || [];
                    this.relatedNews = res.data.value.relatedNews || [];
                }
                this.loadShow = false
            } catch (err) {
                this.loadShow = false
                console.log(err);
            }
        },
        linkToLeague(pageTitle){
            if(pageTitle){
                if(pageTitle.uniformLeagueId ==0 && pageTitle.tournamentId ==0 && pageTitle.leagueId ==0){
                    return 'javascript:void(0)'
                }else {
                    return '/zqlszl/?uniformLeagueId=' + pageTitle.uniformLeagueId + '&tournamentId=' + pageTitle.tournamentId
                }
            }else {
                return ''
            }
        },
        linkToTeam(tid){
            if(tid !='' || tid !=0){
                return '/zqlszl/qdzl/?tid='+tid
            }
            return 'javascript:void(0)'
        },
        // 判断积分榜、射手、伤停是否在前端展示
        getJfbShow(){
            if(this.notShowJcJfbAry.length ==0 && this.notShowWbsjJfbAry.length ==0) this.isShowJfb = true
            if(this.notShowJcShooterAry.length ==0 && this.notShowWbsjShooterAry.length ==0) this.isShowShooter = true
            if(this.notShowJcInjuryAry.length ==0 && this.notShowWbsjInjuryAry.length ==0) this.isShowInjury = true
            if(this.mid !=null && this.notShowJcJfbAry.indexOf(this.mid) > -1){
                this.isShowJfb = false;
            }else{
                if(this.notShowWbsjJfbAry.indexOf(this.gm) > -1) {
                    this.isShowJfb = false;
                }
            }
            if(this.mid !=null && this.notShowJcShooterAry.indexOf(this.mid) > -1){
                this.isShowShooter = false;
            }else{
                if(this.notShowWbsjShooterAry.indexOf(this.gm) > -1) {
                    this.isShowShooter = false;
                }
            }
            if(this.mid !=null && this.notShowJcInjuryAry.indexOf(this.mid) > -1){
                this.isShowInjury = false;
            }else{
                if(this.notShowWbsjInjuryAry.indexOf(this.gm) > -1) {
                    this.isShowInjury = false;
                }
            }
        }
    }
})
app.mount('#app');

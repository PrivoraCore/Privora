/*
This file contains test vectors for firpow (a retuned version of ProgPow 0.9.4)
*/

#pragma once
#ifndef CRYPTO_PROGPOW_PRIVORAPOW_TEST_VECTORS_HPP_
#define CRYPTO_PROGPOW_PRIVORAPOW_TEST_VECTORS_HPP_
namespace 
{
 /// Defines a test case for ProgPoW hash() function.
struct privorapow_hash_test_case
{
    int block_number;
    const char* header_hash_hex;
    const char* boundary_hex;
    const char* nonce_hex;
    const char* mix_hash_hex;
    const char* final_hash_hex;
};   

privorapow_hash_test_case privorapow_hash_test_cases[] = {
{1, "2d794e900dcad779e658de9078d9a88eee87d75f7b09a8fdd270d3a8e76650c7", "0001869e7a058e2aaf266cd2f166fb85c98d651e60eadbbe72bb0a36f8802805", "85f22c9b3cd2f123", "cfab3766331d6c4e6913e6688a71e4c26b7f36c1581cdbec0f5b19db8956eb50", "00017c7de1fa499314f9e3dd3537546982073624f7d478592cf28a6d13929f2d" },
{2, "312d5670db5cb352e232d372df400c050b4966f0f75a358422cd66d49ddac706", "0001869e7a058e2aaf266cd2f166fb85c98d651e60eadbbe72bb0a36f8802805", "85f22c9d3cd3018d", "43c12507b86cbf4530217c9d2552b476d02e813d939c0af415f4deb6442c9880", "0000b9689069f0b245f795683b3dd8c7ac6db3042aaa7c4f41c9d9aa457ce38f" },
{3, "e24e712050309ba90ed700b3f398cd86d4b1f1c3512fd4868160e7b46b163b71", "0001869e7a058e2aaf266cd2f166fb85c98d651e60eadbbe72bb0a36f8802805", "85f22c9d3cd3174e", "40a97d9bc76f9fafba88d282f37c64f4b83b12a10c884ac44da603e3a1d3c954", "0000f005e9038845ec66c43a16dccc231139c7bbe4233b9ff0ac83e23358e039" },
{4, "f8ae4c2ddaf46743ebac31d9cb9297ce4899770f0571ebe8b054c33df4f12f99", "0001869e7a058e2aaf266cd2f166fb85c98d651e60eadbbe72bb0a36f8802805", "85f22c9a3cd2ecee", "5cb1a5886a18720d8c0cd7cc880dec917308d454f01afb3429bdf279a4f119f4", "0000342886ef7663e625e7fd448efafaf3407031b99aa3ad99cea817f9731a91" },
{1299, "e0724517229995bb72bf516a385716627dec523f622c79660a1d27426ca8ad23", "00030d3cf40b1c555e4cd9a5e2cdf70b931aca3cc1d5b77ce576146df100500a", "dec25421bac291d6", "8cff2cf5ae77dff90485a55d1f5a14cc2f0ad0a5dbd129971b8e45a25e01737b", "00021f431adcff9f70154985b469758d7fa97fb0adc33d2f58f856fe19817b4a" },
{1300, "1fc36bd5d1bff8d134e24a997cfa43cbb2a0b956379bdc0c8df444f2553f6b7d", "00030d3cf40b1c555e4cd9a5e2cdf70b931aca3cc1d5b77ce576146df100500a", "dec25420bac29b01", "5f8fe6069efb88d9af861999973b3523295d3b1ec7e8423c965f33b3d12b20b1", "000038a1804eb3976346bf84e679884d56122e113c3b4a301f6180a246644262" },
{1301, "ecbd807e58edaf0fb3ee7c167a7c2728b31cd6a8fd23219afdc5f7be3161c8eb", "00030d3cf40b1c555e4cd9a5e2cdf70b931aca3cc1d5b77ce576146df100500a", "dec25420bac292d8", "30ba0ee0916d403d71cfa78e7fbebca7c31e2e829e977dd18848cb0f22ec5dbf", "00018b40127c8e24ef958d9867b543aec015bb8eb10f77844b49b7451c48bb12" },
{1302, "4cb46ab5c76d0f86e09d71947ec4d403cb95c8f70a335ae2163cfed2ee06bd05", "00030d3cf40b1c555e4cd9a5e2cdf70b931aca3cc1d5b77ce576146df100500a", "dec2541fbac299b5", "01e3ecf528e4f3b40203bfd9651857fa4440b93d5ac5d0c6f7d8d8ccc202014d", "00022678ddd120a3454bc894c139badb5933ae76c5b2c4c2dae2d6c54224ffcf" },
{1303, "0b4410741b34f040065633185af718248093dc66ae9430fbfb69ef023acbf145", "00030d3cf40b1c555e4cd9a5e2cdf70b931aca3cc1d5b77ce576146df100500a", "dec2541fbac2a4e9", "e1e26b162b92599f1c88ef7952b7d265013b7f8e928fe3ff5b051b7c3e6b2a30", "0002b72cba06b7f872af443a826d5409f4d7c9b24ae052324dba403e4d4a8bfa" },
{12999, "0f26f1349755c816aa69090f228f37404c543a717b5d2a060d31000d0ec89140", "0001e84617ccaeb80f88c38f5fa4985fb9a2434e6dd31586e7f5a404cd315b1d", "10d0835770ff398f", "d46f166b0ca83df5c1cda3104b496ebe234d55da1bfb0fa8a630b18bdaa6a504", "0001146b5363e81238e48541cc49fb3cfc27d833126e7b49ad60d7a2facc9bda" },
{13000, "794688e6167995f4891124d488365df76becfd2fc47c5c8337c9a545801d8e60", "0001e84617ccaeb80f88c38f5fa4985fb9a2434e6dd31586e7f5a404cd315b1d", "10d0835970ff1254", "653b09240717ddc1d251e75cd24b5fb35d7890a08be6e2ac6b97105f0ff5b27d", "000131b06b06be7a9bd653065f21e0a2a6e46996849f9f8daa996df35e1a9f0e" },
{13001, "49aac6ff085524ac8e5f62f3d83cf538b8ba1c1ef985c4836a0db732d82f5501", "0001e84617ccaeb80f88c38f5fa4985fb9a2434e6dd31586e7f5a404cd315b1d", "10d0835970ff0be0", "e0a4114afde4f73720f821c033a03783ab4eba3868cd4075fab9c64ccb9961da", "0000ea1289598f14ed43f3a9a6bb3aeadf44f4352df307abac2fa7f4b3f9fbe1" },
{13002, "e6e3cdbbc6998b2b006906f7b13c0638f43c8dcd1b47e4db75df15b19cc06089", "0001e84617ccaeb80f88c38f5fa4985fb9a2434e6dd31586e7f5a404cd315b1d", "10d0835770ff05fa", "68f23dd01fba47c9c4ed8e314512bc5108a858ba990fbd5aece5eb50d5593be8", "00005d7d46632a52e2a232a1ff2c8af11a4a64933ac923c2610535b70275d16e" },
{13003, "eba5065ece50fbe0df889b80b5429b1138ddbce0c880fc98fc41db27c16acc52", "0001e84617ccaeb80f88c38f5fa4985fb9a2434e6dd31586e7f5a404cd315b1d", "10d0835a70ff13e7", "717f4970843d90af5672bef264b14fda176f5114b71bd2903c0d330d2ea64a5c", "00010f49654b470b6568bdce805d13b333f99691b3477b652c5d0c96b31d7fef" },
{13004, "5f4d66eedfabeecdb0840b3a94f670b0aa1637567628a979677b944d53d66350", "0001e84617ccaeb80f88c38f5fa4985fb9a2434e6dd31586e7f5a404cd315b1d", "10d0835870ff14cc", "9c64f92a4a87e2355e05ed307ce3c68ecafa33b78c4a058922a549202a96f832", "0001d2e8ea914a696f9cacafd7c6996d0d06b86ff1f00ae97d8947d70bfe5176" },
{13005, "1a75e51b3c24968ff80af1458872bc2bf7fbd6b7a68663d815299369d1d2b49f", "0001e84617ccaeb80f88c38f5fa4985fb9a2434e6dd31586e7f5a404cd315b1d", "10d0835970ff0928", "384428101d1394a2943672077d8edf771e2ddb4b412a97cd4cd3c8497d800285", "0000a463b128e65c08cc4c668b8d35d1740ec0ed5a18344305d7fe939d81e4f4" },
{13006, "7072ca2d6172ae53aed24ec54db19c38f2682b1f3365276237ea1bfb3e2adcfc", "0001e84617ccaeb80f88c38f5fa4985fb9a2434e6dd31586e7f5a404cd315b1d", "10d0835870ff9174", "19892a71a2e2de646e80571e287f9a1f2561b7bf30f99052dd9080bc12287a32", "0000d79a1627cc9d8d1f55dd93dc03badfebbdda82dc12df073d40ff7cb344b4" },
{13007, "94c6060f7843dca66edcdd1f752f32c1b9fbf9a523780222ced9748688fba511", "0001e84617ccaeb80f88c38f5fa4985fb9a2434e6dd31586e7f5a404cd315b1d", "10d0835770ff0291", "26ff0482fd2d2984cd2a43c3ebf6ed043c5f20df87cabafcce0f880a84674718", "000154031e70c22bd074023d9f2d15d9609a172c77c6693acbb5ded8842c5db6" },
{13008, "f962572c9663cd7986c2560449afa3595d10a290525caf060c65dcef56422659", "0001e84617ccaeb80f88c38f5fa4985fb9a2434e6dd31586e7f5a404cd315b1d", "10d0835870ff4802", "ad9bba3d9baa53cf832d25d3afe85c68c3e0baa796c0450e79e17cff8e921cf7", "0000e4891befecbff65859f0aef391b70771acc4196afa30c4ca53ce03bf0dd1" },
{13009, "17a943d97e7a157eed35d0a63a1c72a8d364d95b491706eedd7a22da2adceab4", "0001e84617ccaeb80f88c38f5fa4985fb9a2434e6dd31586e7f5a404cd315b1d", "10d0835a70ff28f5", "19263eb69773ba960ed7fddd64e6802101db5e5445ee848e3765cb577fafa13a", "00001c83042832b92722b4049c591a17ee2fef150ccbe2c1e256f5c22cb4df2e" },
{13010, "8c3715e03eb354ccbf9cd846c97e7b88b0f1d11dfa7efb2c8472dc9885b72856", "0001e84617ccaeb80f88c38f5fa4985fb9a2434e6dd31586e7f5a404cd315b1d", "10d0835a70ff0934", "0652be5c9c43fd63923cca59dd614e90bbc672c20b493a98c6abbe8ff5f588f8", "0000bb57589af1269c7c65c30d957e47417139cc735610ee6a8191ffc9219f15" },
{13011, "d227eae293a153735775b2cc3622636bdd3aa5da061734bc1659893a7ae35f57", "0001e84617ccaeb80f88c38f5fa4985fb9a2434e6dd31586e7f5a404cd315b1d", "10d0835870ff21f9", "6632515805e6416845e8c2b139f2d3b59c0e24acfeec46e154a42b584253f714", "00009c64ce1d8970f17495ba376eded9958f5dd29e2460ef67dfc1fc613387f0" },
{13012, "431b7cc510b0c16040406006f617833caf00191910763470dfd5af0fd2da2607", "0001e84617ccaeb80f88c38f5fa4985fb9a2434e6dd31586e7f5a404cd315b1d", "10d0835970ff09e1", "a86156101ca1260fcf4ce609529b871b8eb14035e9bb2b0cd09ac30a53c304c8", "00000184b4f8e57b0fb8147e83275d5f12784e137b45e98c5dbca6bdffd2174b" },
{48650, "b1e6202f07ef291b18497c8210639f246d43299c27d1354bb959430b20cebf12", "00004e1fb2011c6ef320e5a5fc1b76807358965ce1ccf5fcc5dda026cd038963", "f63c144f8f7a580c", "419d0df63ea7576a3ee8ca2bc226edc193b50caa086aa51b772b59ef805e7cc5", "00004a35ac80a9ccca61baed18a7df446b56b4c7816b928d6c530e2212cb426c" },
{48651, "d1175e6a053cd3f346f3c51c144055c869198f5f52531ab8f93a039fee2dd6fb", "00004e1fb2011c6ef320e5a5fc1b76807358965ce1ccf5fcc5dda026cd038963", "f63c14528f7d6a63", "ef010595bbd8ed93718a006f33789751a86897a45bbdfc3dd759c7825ae3f4e1", "000042e351bf4f2bd4029a124f71495c520f1450ceba60d4e10b6b8df203598c" },
{48652, "ccef7f9c32178c6cc602472decbf4e7fd94e040170083d5f12021b2b3d71f104", "00004e1fb2011c6ef320e5a5fc1b76807358965ce1ccf5fcc5dda026cd038963", "f63c144f8f7a5e25", "126f97c6f53f3122e735f7e7bfbadce009c189a4719d3528f58c875af0c4f995", "0000208614e2c5f10af77ed5dd4a39eb18d65142bd8494a948691924424d7bcd" },
{48653, "2c128024a0274ec45f773fa878e0f9efc309ebc4864e63346931fb0a80ec9f1e", "00004e1fb2011c6ef320e5a5fc1b76807358965ce1ccf5fcc5dda026cd038963", "f63c14518f7a9067", "6d64d93fa9bdd7f3d1a0dd9a18d32293bc665f196017284e4a2fdb7a24e1a2f4", "000045e2e42b17acd3b394c3b13e68ac466366d96f3f859fcda479cf038e3aa5" },
{48654, "e735c300f9475fbb068e10a6a5525af6ed23a9d722b824560376de5f63a57616", "00004e1fb2011c6ef320e5a5fc1b76807358965ce1ccf5fcc5dda026cd038963", "f63c14508f7c08ca", "db395ee0366eac32d6e9abe6fc4fbcd3a1010263ae0c6a1a3e17a217b22c23d3", "000014d06665181b73056cca8426f9c396f7c4e3f5556b17289c0c3079f730ac" },
{48655, "bf0a109b79b2e989590e7a12acccc4d79016e0d95ed73f15170dfc446917552d", "00004e1fb2011c6ef320e5a5fc1b76807358965ce1ccf5fcc5dda026cd038963", "f63c144f8f7a760d", "b3285062764170d185f90e79ae315821eb97e556e99761078a07028da478710e", "000020d490bc8079daeb3454c0546272382cdc8539e5927b73e3681ee53c0494" },
{48656, "0033058cb7a6b4b4a31ac1f5755573a542fa121e2aec6b4b8053ffeb5d25d5cf", "00004e1fb2011c6ef320e5a5fc1b76807358965ce1ccf5fcc5dda026cd038963", "f63c14518f7cf884", "f31c21fa4fa526d8cd61648b75abd7bcad95fbb6abdb552897838a0d699b8aac", "0000114f87c0ab69515ddcc536c2800ffb9000d3d6e92a80f1f4528dc97feb62" },
{48657, "70517a1e542e8706d22b47f7350dd0940edeec87b15cf8f47fd4f39d38946d5f", "00004e1fb2011c6ef320e5a5fc1b76807358965ce1ccf5fcc5dda026cd038963", "f63c144f8f7a6b4d", "501ffa55559d938660ddff613bb0ecf46f253b45530d3bde7eb4a2fb6cf2d154", "0000465f1a28272e53680cfd4c9962ee6cfed960580cf82afb981fa75ab391e6" },
{48658, "0de3db810d0384a17fbdca43c8153fe56b6253e2d83d167c93a562ab6ed9d552", "00004e1fb2011c6ef320e5a5fc1b76807358965ce1ccf5fcc5dda026cd038963", "f63c144f8f7a5219", "a41d1e701aa411e56ea8d519be9a49f302ddbcb29969ecf235c751167f0fce22", "0000345f4413afee14d84ba3b155f0b39d841cf3c75cd961e06a5dbedab52478" },
{48659, "15a8044fb91d7e51a71a0e4192b7a348b27d23742343dfa3f26fd3b1fda53309", "00004e1fb2011c6ef320e5a5fc1b76807358965ce1ccf5fcc5dda026cd038963", "f63c14528f7a8441", "1357cb9ed8339cf39f3293298aa85f9fe6d25b8f0706a74c34d9c25db5253f77", "00000f68bb4cad82f2e113ef54278b7601b9553c764e21b5f7121d8656e6d151" },
{48660, "5053fcbea728d7c4c3ba2e8ddced295aed66a7a79921b6d3740589accf75bcbe", "00004e1fb2011c6ef320e5a5fc1b76807358965ce1ccf5fcc5dda026cd038963", "f63c14508f7a8541", "6153328c35d1118f48b5ef4e52e9b7ebebe45e03f8a540ac6c15cd1efff66964", "00004710ef9f5f7b1ecaec1f75453a8240ca3b5c8af5b379c1417881d9baf890" },
{48661, "0abf147cc465d649f6a2369f43fc48b40ff8c5c2bbf4d11b7539114169dbbc2c", "00004e1fb2011c6ef320e5a5fc1b76807358965ce1ccf5fcc5dda026cd038963", "f63c144f8f7a5152", "2984861e2add6efa0fd7fff989eca7d8d636626de0accc467aef4a391d57859b", "0000402da9d2e2576c559032944a4631ce45f02710147987072262ddcc95b2b6" },
{96587, "80b9cd8b67db95fdb900affece4d7d2c14ddec9b901be0d6e42b89fc7819c693", "0000270fd9008e377a1d2f124ab10259b51cbebce8a2f200089e951808ed04ec", "6fec0853f96eb6f3", "3a9608a1f415a76a98eb4fbcb2285db14d3e9d411aacb6a89525432b1b228132", "00001e5086d2148fec8df3097580efb2672786bca9fe7dba04cd111f21cd42de" },
{96588, "f6737c8737818d6d69078554279f55576102ca182290d263395eb2b7ca3df5f4", "0000270fd9008e377a1d2f124ab10259b51cbebce8a2f200089e951808ed04ec", "6fec0855f96a97e5", "bb6ecae5ae70526d0f35888c11f34cc0095db347e80fe4f2b544efe93aa82db8", "000002dc8b7d376e2ab354af218d96cb29c6869098cd22aeefd3ef3fea0489d8" },
{96589, "15fff750c0be3b1154b84adf42402ccd068c82c3103ea4e72ea100b49e3b51fe", "0000270fd9008e377a1d2f124ab10259b51cbebce8a2f200089e951808ed04ec", "6fec0854f96d1416", "542db538572570227f3bdcec56c45788b83da23eeea3cd1edae5ec62df6761cf", "00000b981f0a57e10cbd39096fb27341f21dbb568861bca06efdf6619b563b13" },
{96590, "0231f310fb1a0cfd63f363802ae3f0a90c50b3b879d8cedb243efc1446d0d6b2", "0000270fd9008e377a1d2f124ab10259b51cbebce8a2f200089e951808ed04ec", "6fec0856f969dd81", "a91de8ff0849aed667d4e11f610e3fdc4612ea7d8fe6c58e902eaf95c978bf54", "000016f6ebb36334946238e06d484b8ed8ff50477729eb28c4c294a9e8218f97" },
{96591, "c5cf3e468cf1987809c7ec07a0d54abead5e843c35cb1fd9088b35b55f4a1d28", "0000270fd9008e377a1d2f124ab10259b51cbebce8a2f200089e951808ed04ec", "6fec0856f96c094a", "37aa1832f41ccb16ca3956148034bcfe6c55fcc41fe835bdca99e1ca41a0bda5", "000008bdb0253bbdef6d0c29b8b840ecd955b4f096fcca1f251ba3060b1b1853" },
{138524, "c3245ec955bd49ce5994a800f6c14db5c17fcc6ccc490f77847bbb7381f4f779", "000007cff829c6463eb5a6e895793e95b86217ccbcab60bf97784f3100f2685e", "3ddce1e1713d2952", "962c9ff57906f6b13e4cbd6d9dd5d0ace43dc69013b49370301cd02a9c90ef11", "0000003bcd31ed7c2341af79941419da4475593cd4eadfeb2c7f1392f36fc1f1" },
{138525, "a4b15a6aa6a61b36b53a4fcba5d0f91deda619a1b639c41e3e796de66d0ece97", "000007cff829c6463eb5a6e895793e95b86217ccbcab60bf97784f3100f2685e", "3ddce1e471397224", "0d92269db895547d47c307219aee739267998c7115c510bb94e50ce75eb6b642", "000005ffdbc2df46b9967115692827e5701a0953d8dfb33375a0e2e7665e8c01" },
{265000, "5a085fb8be7e0f10cbeb45a1deda25abfef270e12a203c7dfb020aac0723fa7c", "0001869e7a058e2aaf266cd2f166fb85c98d651e60eadbbe72bb0a36f8802805", "f3e95657f2470e38", "b5500ea9243f46c0ec2a2293dd5c5b49d299395635597597405f3a87225b472f", "00010c328e59ca58ad0fa9c7f0d266c3dc6cfa83e77bb44e92eb9aa2f377abb6" },
{265001, "6071af1007725ee11f28b1f010a1ac0cafbda917947e9737592197f0808290af", "0001869e7a058e2aaf266cd2f166fb85c98d651e60eadbbe72bb0a36f8802805", "f3e95655f247313c", "b5a71090dc07218d392a56cbc55c08b9b616d53a0cdc1c166ae1f91ea39c2a6c", "0001269e4b345375261bb78a19a513f536a12e5882c1a7d1152cc8c2df7e8011" },
{265002, "2ebef575a00ff895cbec8774cf670e2514c05ec1eb0609e740a8522813cad487", "0001869e7a058e2aaf266cd2f166fb85c98d651e60eadbbe72bb0a36f8802805", "f3e95657f2472f25", "47d8c49c60cd88ea187fcf2eee162b0506606e937a01953f65de54ff4358b2bc", "0000b4760f61f4a37e9a9eb01407b5846c3d27a09dc6b5e7bcfd5c1357f040f4" },
{265003, "91b662e2355d7fd83dc847575aa47e321c852bd4105792a85e76f7e5f44e3c90", "0001869e7a058e2aaf266cd2f166fb85c98d651e60eadbbe72bb0a36f8802805", "f3e95658f2474f6b", "94ab3bf6300ae2b2d336f1ec0e6bc482a9736e4740b2bc11bd950be131e7a4fe", "00013b8edb98563fa532445437d2f1e25ca7c8cff3fbc28104fe9cfa1567ef5b" },
{400000, "df8208e69e7c8eac0121fc6a9377ae0790079352714fbffdb3c4ec13f1bfc884", "0001869e7a058e2aaf266cd2f166fb85c98d651e60eadbbe72bb0a36f8802805", "32ec0f0a8878e9c1", "65fd5ae460458787790c5c3f07676d9ebde9209625b4069b1f54f8d6ae12c668", "0000d009a21a524e53b49b4776a269549e88b86a90afca329b8f2e7ae70e3776" },
{400001, "b98bcec3991c33a1211d49360cd90c0906652240a56668530bde843e9cfd1703", "0001869e7a058e2aaf266cd2f166fb85c98d651e60eadbbe72bb0a36f8802805", "32ec0f0b88790fbe", "c5e943a63ffe8f68eba2b1dcc9fcea7044c5b76c0cd4654ea84ce455f2cd5642", "00006e2a6f617b6de63500382092988d292283d42cb437ade47c1f20f11f0998" },
{400002, "e220a0c62442740ad95b1b941b054226d720543b3dcdbfae99a72c9ebe71709d", "0001869e7a058e2aaf266cd2f166fb85c98d651e60eadbbe72bb0a36f8802805", "32ec0f0a8878ee2e", "d497b31427a5ab4f90fea12b46374be01c4fa9a72e8c2e9e39352ee3167f680a", "0001601a6d05af98a48453e8320a5aa408fd0a79ad2a6e8d630128406614ca9d" },
{400003, "1ac721ef41f48e58d1357f9e1ac588eecf59e4620c5cf1e360e888171b7f6f97", "0001869e7a058e2aaf266cd2f166fb85c98d651e60eadbbe72bb0a36f8802805", "32ec0f0988792f53", "10184e6c54293704d07d836334c924eb3aee47fd28a2afe68f5cff1e7ee0ece2", "00006a95e279f56df10a9aff98dceb141b83615050dcc1de0a7a9a5daa27f5e2" },
{545856, "4741563174fb16f3abf2ca9b593e7f2cca7d5363cce3cb6b1792a9fc97b6848a", "00004e1fb2011c6ef320e5a5fc1b76807358965ce1ccf5fcc5dda026cd038963", "844c83fe15de8d05", "a0fee97a41a7bd84a7e2d75a9cac1dd66f0b3ace888f029e2102d14d5f64b09d", "0000305bf56700d0c304b91a57b6564eaa7815926fdaf89e5133fa29e4300ab0" },
{545857, "c09cb3f75345751eb84e35a71712123d7f0c8e2edbb980be29034a1aadda0725", "00004e1fb2011c6ef320e5a5fc1b76807358965ce1ccf5fcc5dda026cd038963", "844c83fd15dea41f", "46e2dc0f08e4ffbc8cc8a0355676c2e62d9922ec41a3aa27cc1d6948acca543c", "00001cac52c8dafcb697b0fa6257b81891242ec0f89910c15d57629ec31b69c8" },
{545858, "802730918e3776de37b0b03e5f9e1ebd5c7376097e64578a7702a647214432b3", "00004e1fb2011c6ef320e5a5fc1b76807358965ce1ccf5fcc5dda026cd038963", "844c83fd15de3317", "66d917802bf756031404d6947e4b389db703af51df2b72dc7806e339a0e51961", "00001d63aff2cee40464b7ea580cbc44ae2842a961dfd78f4084ab99179beb30" },
{545859, "c09e4a8e697fb29447a5301b4c716182a267020b131a0f4137c1841e4c6fc8a2", "00004e1fb2011c6ef320e5a5fc1b76807358965ce1ccf5fcc5dda026cd038963", "844c83fc15de0838", "768f7232065b7d6ae6e2db50b88b8bb8ec5df6dc9dee45ed95850586c60904f3", "0000486174a163e26818c06f6069ee8e16ff66dd26f3e3bcf7bf3c964f999590" },
{545860, "421ecabe666b8a4b3c5d9f899a15115f1fc8e2644468459c9f200d2dd10c204c", "00004e1fb2011c6ef320e5a5fc1b76807358965ce1ccf5fcc5dda026cd038963", "844c83fd15ddbc98", "19b62e800a00b6ad8208eb53baeebe9d5e96018ccc7df2ebe35cac463984ea4b", "0000368b42a54273789090266c5093f660642e728031448cc6b21b1ca7ebcbb7" }
};
} // namespace 

#endif // !CRYPTO_PROGPOW_PRIVORAPOW_TEST_VECTORS_HPP_

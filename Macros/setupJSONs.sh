#!/bin/bash

. $CMSSW_BASE/src/UserCode/Stop4Body/Macros/setupPaths.sh

touch $JSON_PATH/tmp.json
rm    $JSON_PATH/*.json

# Make a JSON for all MC except signal
mergeJSONs $JSON_PATH/Orig/Wjets.json         $JSON_PATH/Orig/Wjets_*.json
mergeJSONs $JSON_PATH/backgroundMC.json       $JSON_PATH/Orig/TTbar.json $JSON_PATH/Orig/Wjets.json
cp         $JSON_PATH/Orig/Wjets.json         $JSON_PATH/Wjets.json
cp         $JSON_PATH/Orig/TTbar.json         $JSON_PATH/TTbar.json
cp         $JSON_PATH/Orig/TTbar_LO.json      $JSON_PATH/TTbar_LO.json
cp         $JSON_PATH/Orig/TTLep.json         $JSON_PATH/TTLep.json
mergeJSONs $JSON_PATH/TT_pow.json             $JSON_PATH/Orig/TT_pow.json $JSON_PATH/Orig/TT_pow_backup.json
#mergeJSONs $JSON_PATH/otherMC1.json           $JSON_PATH/Orig/QCD.json
#mergeJSONs $JSON_PATH/otherMC2.json           $JSON_PATH/Orig/ZInv.json
#mergeJSONs $JSON_PATH/otherMC3.json           $JSON_PATH/Orig/VV.json       $JSON_PATH/Orig/SingleTop.json
#mergeJSONs $JSON_PATH/otherMC4.json           $JSON_PATH/Orig/DYJets.json
mergeJSONs $JSON_PATH/otherMC1.json           $JSON_PATH/Orig/QCD.json $JSON_PATH/Orig/TTGJets.json $JSON_PATH/Orig/TTW_LO.json $JSON_PATH/Orig/TTZ_LO.json $JSON_PATH/Orig/TTWToLNu.json $JSON_PATH/Orig/TTWToQQ.json $JSON_PATH/Orig/TTZToQQ.json $JSON_PATH/Orig/TTZToLLNuNu.json
mergeJSONs $JSON_PATH/ttx.json                $JSON_PATH/Orig/TTGJets.json $JSON_PATH/Orig/TTW_LO.json $JSON_PATH/Orig/TTZ_LO.json $JSON_PATH/Orig/TTWToLNu.json $JSON_PATH/Orig/TTWToQQ.json $JSON_PATH/Orig/TTZToQQ.json $JSON_PATH/Orig/TTZToLLNuNu.json
cp         $JSON_PATH/Orig/ZInv.json          $JSON_PATH/otherMC2.json
mergeJSONs $JSON_PATH/otherMC3.json           $JSON_PATH/Orig/VV.json      $JSON_PATH/Orig/SingleTop.json
cp         $JSON_PATH/Orig/DYJets.json        $JSON_PATH/otherMC4.json
mergeJSONs $JSON_PATH/otherMC.json            $JSON_PATH/Orig/QCD.json $JSON_PATH/Orig/TTGJets.json $JSON_PATH/Orig/TTWToLNu.json $JSON_PATH/Orig/TTWToQQ.json $JSON_PATH/Orig/TTZToQQ.json $JSON_PATH/Orig/TTZToLLNuNu.json $JSON_PATH/otherMC2.json $JSON_PATH/otherMC3.json $JSON_PATH/otherMC4.json
mergeJSONs $JSON_PATH/allMC.json              $JSON_PATH/backgroundMC.json $JSON_PATH/TT_pow.json $JSON_PATH/TTbar_LO.json $JSON_PATH/TTLep.json $JSON_PATH/otherMC1.json $JSON_PATH/otherMC2.json $JSON_PATH/otherMC3.json $JSON_PATH/otherMC4.json
mergeJSONs $JSON_PATH/allMCQuick.json         $JSON_PATH/Orig/Wjets.json $JSON_PATH/otherMC1.json $JSON_PATH/otherMC3.json
#$JSON_PATH/TTLep.json $JSON_PATH/otherMC2.json $JSON_PATH/otherMC4.json

#Make a JSON with the samples that are to be processed:
mergeJSONs $JSON_PATH/MC2Process.json         $JSON_PATH/Orig/Wjets.json $JSON_PATH/Orig/TTbar.json $JSON_PATH/Orig/TTbar_LO.json $JSON_PATH/Orig/TTLep.json $JSON_PATH/TT_pow.json $JSON_PATH/Orig/QCD.json $JSON_PATH/Orig/ZInv.json $JSON_PATH/Orig/VV.json $JSON_PATH/Orig/SingleTop.json $JSON_PATH/Orig/DYJets.json $JSON_PATH/Orig/TTGJets.json $JSON_PATH/Orig/TTW_LO.json $JSON_PATH/Orig/TTZ_LO.json $JSON_PATH/Orig/TTWToLNu.json $JSON_PATH/Orig/TTWToQQ.json $JSON_PATH/Orig/TTZToQQ.json $JSON_PATH/Orig/TTZToLLNuNu.json

# Copy the data JSON over, so that all locations are consistent
cp         $JSON_PATH/Orig/Data.json          $JSON_PATH/
#mergeJSONs $JSON_PATH/DataLepton.json         $JSON_PATH/Orig/DataSingleElectron.json $JSON_PATH/Orig/DataSingleMuon.json
cp         $JSON_PATH/Orig/DataSingleLepton.json $JSON_PATH/DataSingleLepton.json

# Make a JSON for all PD
mergeJSONs $JSON_PATH/allPD.json              $JSON_PATH/Orig/PD.json       $JSON_PATH/Orig/PDInj.json

# Make a JSON for producing the PD
mergeJSONs $JSON_PATH/makePD.json             $JSON_PATH/Orig/TT_pow.json   $JSON_PATH/Orig/Wjets.json $JSON_PATH/Orig/ZInv.json
mergeJSONs $JSON_PATH/makePD_Inj_300_270.json $JSON_PATH/makePD.json        $JSON_PATH/Orig/SignalPoints/Sig_300_270.json

# Make the JSON for synchronization
mergeJSONs $JSON_PATH/synch.json              $JSON_PATH/Orig/TTbar.json    $JSON_PATH/Orig/SignalPoints/Sig_300_270.json

# Make the JSONs for plotting
mergeJSONs $JSON_PATH/plot2016.json           $JSON_PATH/otherMC.json       $JSON_PATH/Orig/TTbar.json    $JSON_PATH/Orig/Wjets.json  $JSON_PATH/Orig/Data.json         $JSON_PATH/Orig/SignalPoints/Sig_300_270.json
mergeJSONs $JSON_PATH/plot2016_pow.json       $JSON_PATH/otherMC.json       $JSON_PATH/Orig/TT_pow.json   $JSON_PATH/Orig/Wjets.json  $JSON_PATH/Orig/Data.json         $JSON_PATH/Orig/SignalPoints/Sig_300_270.json
mergeJSONs $JSON_PATH/plot2016_lep.json       $JSON_PATH/otherMC.json       $JSON_PATH/Orig/TTLep.json    $JSON_PATH/Orig/Wjets.json  $JSON_PATH/Orig/Data.json         $JSON_PATH/Orig/SignalPoints/Sig_300_270.json
mergeJSONs $JSON_PATH/plot2016swap.json           $JSON_PATH/otherMC.json       $JSON_PATH/Orig/TTbar.json    $JSON_PATH/Orig/Wjets.json  $JSON_PATH/Orig/DataSingleLepton.json $JSON_PATH/Orig/SignalPoints/Sig_300_270.json
mergeJSONs $JSON_PATH/plot2016swap_pow.json       $JSON_PATH/otherMC.json       $JSON_PATH/Orig/TT_pow.json   $JSON_PATH/Orig/Wjets.json  $JSON_PATH/Orig/DataSingleLepton.json $JSON_PATH/Orig/SignalPoints/Sig_300_270.json
mergeJSONs $JSON_PATH/plot2016swap_lep.json       $JSON_PATH/otherMC.json       $JSON_PATH/Orig/TTLep.json    $JSON_PATH/Orig/Wjets.json  $JSON_PATH/Orig/DataSingleLepton.json $JSON_PATH/Orig/SignalPoints/Sig_300_270.json

mergeJSONs $JSON_PATH/plot2016_DM10RP.json     $JSON_PATH/otherMC.json       $JSON_PATH/Orig/TTLep.json    $JSON_PATH/Orig/Wjets.json  $JSON_PATH/Orig/Data.json         $JSON_PATH/Orig/SignalPoints/Sig_475_465.json
mergeJSONs $JSON_PATH/plot2016swap_DM10RP.json $JSON_PATH/otherMC.json       $JSON_PATH/Orig/TTLep.json    $JSON_PATH/Orig/Wjets.json  $JSON_PATH/Orig/DataSingleLepton.json $JSON_PATH/Orig/SignalPoints/Sig_475_465.json
mergeJSONs $JSON_PATH/plot2016_DM20RP.json     $JSON_PATH/otherMC.json       $JSON_PATH/Orig/TTLep.json    $JSON_PATH/Orig/Wjets.json  $JSON_PATH/Orig/Data.json         $JSON_PATH/Orig/SignalPoints/Sig_500_480.json
mergeJSONs $JSON_PATH/plot2016swap_DM20RP.json $JSON_PATH/otherMC.json       $JSON_PATH/Orig/TTLep.json    $JSON_PATH/Orig/Wjets.json  $JSON_PATH/Orig/DataSingleLepton.json $JSON_PATH/Orig/SignalPoints/Sig_500_480.json
mergeJSONs $JSON_PATH/plot2016_DM30RP.json     $JSON_PATH/otherMC.json       $JSON_PATH/Orig/TTLep.json    $JSON_PATH/Orig/Wjets.json  $JSON_PATH/Orig/Data.json         $JSON_PATH/Orig/SignalPoints/Sig_550_520.json
mergeJSONs $JSON_PATH/plot2016swap_DM30RP.json $JSON_PATH/otherMC.json       $JSON_PATH/Orig/TTLep.json    $JSON_PATH/Orig/Wjets.json  $JSON_PATH/Orig/DataSingleLepton.json $JSON_PATH/Orig/SignalPoints/Sig_550_520.json
mergeJSONs $JSON_PATH/plot2016_DM40RP.json     $JSON_PATH/otherMC.json       $JSON_PATH/Orig/TTLep.json    $JSON_PATH/Orig/Wjets.json  $JSON_PATH/Orig/Data.json         $JSON_PATH/Orig/SignalPoints/Sig_550_510.json
mergeJSONs $JSON_PATH/plot2016swap_DM40RP.json $JSON_PATH/otherMC.json       $JSON_PATH/Orig/TTLep.json    $JSON_PATH/Orig/Wjets.json  $JSON_PATH/Orig/DataSingleLepton.json $JSON_PATH/Orig/SignalPoints/Sig_550_510.json
mergeJSONs $JSON_PATH/plot2016_DM50RP.json     $JSON_PATH/otherMC.json       $JSON_PATH/Orig/TTLep.json    $JSON_PATH/Orig/Wjets.json  $JSON_PATH/Orig/Data.json         $JSON_PATH/Orig/SignalPoints/Sig_525_475.json
mergeJSONs $JSON_PATH/plot2016swap_DM50RP.json $JSON_PATH/otherMC.json       $JSON_PATH/Orig/TTLep.json    $JSON_PATH/Orig/Wjets.json  $JSON_PATH/Orig/DataSingleLepton.json $JSON_PATH/Orig/SignalPoints/Sig_525_475.json
mergeJSONs $JSON_PATH/plot2016_DM60RP.json     $JSON_PATH/otherMC.json       $JSON_PATH/Orig/TTLep.json    $JSON_PATH/Orig/Wjets.json  $JSON_PATH/Orig/Data.json         $JSON_PATH/Orig/SignalPoints/Sig_525_465.json
mergeJSONs $JSON_PATH/plot2016swap_DM60RP.json $JSON_PATH/otherMC.json       $JSON_PATH/Orig/TTLep.json    $JSON_PATH/Orig/Wjets.json  $JSON_PATH/Orig/DataSingleLepton.json $JSON_PATH/Orig/SignalPoints/Sig_525_465.json
mergeJSONs $JSON_PATH/plot2016_DM70RP.json     $JSON_PATH/otherMC.json       $JSON_PATH/Orig/TTLep.json    $JSON_PATH/Orig/Wjets.json  $JSON_PATH/Orig/Data.json         $JSON_PATH/Orig/SignalPoints/Sig_525_455.json
mergeJSONs $JSON_PATH/plot2016swap_DM70RP.json $JSON_PATH/otherMC.json       $JSON_PATH/Orig/TTLep.json    $JSON_PATH/Orig/Wjets.json  $JSON_PATH/Orig/DataSingleLepton.json $JSON_PATH/Orig/SignalPoints/Sig_525_455.json
mergeJSONs $JSON_PATH/plot2016_DM80RP.json     $JSON_PATH/otherMC.json       $JSON_PATH/Orig/TTLep.json    $JSON_PATH/Orig/Wjets.json  $JSON_PATH/Orig/Data.json         $JSON_PATH/Orig/SignalPoints/Sig_500_420.json
mergeJSONs $JSON_PATH/plot2016swap_DM80RP.json $JSON_PATH/otherMC.json       $JSON_PATH/Orig/TTLep.json    $JSON_PATH/Orig/Wjets.json  $JSON_PATH/Orig/DataSingleLepton.json $JSON_PATH/Orig/SignalPoints/Sig_500_420.json

# Make a JSON for the PU tests
mergeJSONs $JSON_PATH/puTests.json            $JSON_PATH/Orig/TTbar_LO.json $JSON_PATH/Orig/Data.json

# Copy the JSON with the info for each deltaM over, so that all locations are consistent
cp         $JSON_PATH/Orig/DeltaM.json        $JSON_PATH/

# Make a JSON for each group of signal points with the same stop mass
mergeJSONs $JSON_PATH/stop250.json            $JSON_PATH/Orig/SignalPoints/Sig_250_170.json $JSON_PATH/Orig/SignalPoints/Sig_250_180.json $JSON_PATH/Orig/SignalPoints/Sig_250_190.json $JSON_PATH/Orig/SignalPoints/Sig_250_200.json $JSON_PATH/Orig/SignalPoints/Sig_250_210.json $JSON_PATH/Orig/SignalPoints/Sig_250_220.json $JSON_PATH/Orig/SignalPoints/Sig_250_230.json $JSON_PATH/Orig/SignalPoints/Sig_250_240.json
mergeJSONs $JSON_PATH/stop275.json            $JSON_PATH/Orig/SignalPoints/Sig_275_195.json $JSON_PATH/Orig/SignalPoints/Sig_275_205.json $JSON_PATH/Orig/SignalPoints/Sig_275_215.json $JSON_PATH/Orig/SignalPoints/Sig_275_225.json $JSON_PATH/Orig/SignalPoints/Sig_275_235.json $JSON_PATH/Orig/SignalPoints/Sig_275_245.json $JSON_PATH/Orig/SignalPoints/Sig_275_255.json $JSON_PATH/Orig/SignalPoints/Sig_275_265.json
mergeJSONs $JSON_PATH/stop300.json            $JSON_PATH/Orig/SignalPoints/Sig_300_220.json $JSON_PATH/Orig/SignalPoints/Sig_300_230.json $JSON_PATH/Orig/SignalPoints/Sig_300_240.json $JSON_PATH/Orig/SignalPoints/Sig_300_250.json $JSON_PATH/Orig/SignalPoints/Sig_300_260.json $JSON_PATH/Orig/SignalPoints/Sig_300_270.json $JSON_PATH/Orig/SignalPoints/Sig_300_280.json $JSON_PATH/Orig/SignalPoints/Sig_300_290.json
mergeJSONs $JSON_PATH/stop325.json            $JSON_PATH/Orig/SignalPoints/Sig_325_245.json $JSON_PATH/Orig/SignalPoints/Sig_325_255.json $JSON_PATH/Orig/SignalPoints/Sig_325_265.json $JSON_PATH/Orig/SignalPoints/Sig_325_275.json $JSON_PATH/Orig/SignalPoints/Sig_325_285.json $JSON_PATH/Orig/SignalPoints/Sig_325_295.json $JSON_PATH/Orig/SignalPoints/Sig_325_305.json $JSON_PATH/Orig/SignalPoints/Sig_325_315.json
mergeJSONs $JSON_PATH/stop350.json            $JSON_PATH/Orig/SignalPoints/Sig_350_270.json $JSON_PATH/Orig/SignalPoints/Sig_350_280.json $JSON_PATH/Orig/SignalPoints/Sig_350_290.json $JSON_PATH/Orig/SignalPoints/Sig_350_300.json $JSON_PATH/Orig/SignalPoints/Sig_350_310.json $JSON_PATH/Orig/SignalPoints/Sig_350_320.json $JSON_PATH/Orig/SignalPoints/Sig_350_330.json $JSON_PATH/Orig/SignalPoints/Sig_350_340.json
mergeJSONs $JSON_PATH/stop375.json            $JSON_PATH/Orig/SignalPoints/Sig_375_295.json $JSON_PATH/Orig/SignalPoints/Sig_375_305.json $JSON_PATH/Orig/SignalPoints/Sig_375_315.json $JSON_PATH/Orig/SignalPoints/Sig_375_325.json $JSON_PATH/Orig/SignalPoints/Sig_375_335.json $JSON_PATH/Orig/SignalPoints/Sig_375_345.json $JSON_PATH/Orig/SignalPoints/Sig_375_355.json $JSON_PATH/Orig/SignalPoints/Sig_375_365.json
mergeJSONs $JSON_PATH/stop400.json            $JSON_PATH/Orig/SignalPoints/Sig_400_320.json $JSON_PATH/Orig/SignalPoints/Sig_400_330.json $JSON_PATH/Orig/SignalPoints/Sig_400_340.json $JSON_PATH/Orig/SignalPoints/Sig_400_350.json $JSON_PATH/Orig/SignalPoints/Sig_400_360.json $JSON_PATH/Orig/SignalPoints/Sig_400_370.json $JSON_PATH/Orig/SignalPoints/Sig_400_380.json $JSON_PATH/Orig/SignalPoints/Sig_400_390.json
mergeJSONs $JSON_PATH/stop425.json            $JSON_PATH/Orig/SignalPoints/Sig_425_345.json $JSON_PATH/Orig/SignalPoints/Sig_425_355.json $JSON_PATH/Orig/SignalPoints/Sig_425_365.json $JSON_PATH/Orig/SignalPoints/Sig_425_375.json $JSON_PATH/Orig/SignalPoints/Sig_425_385.json $JSON_PATH/Orig/SignalPoints/Sig_425_395.json $JSON_PATH/Orig/SignalPoints/Sig_425_405.json $JSON_PATH/Orig/SignalPoints/Sig_425_415.json
mergeJSONs $JSON_PATH/stop450.json            $JSON_PATH/Orig/SignalPoints/Sig_450_370.json $JSON_PATH/Orig/SignalPoints/Sig_450_380.json $JSON_PATH/Orig/SignalPoints/Sig_450_390.json $JSON_PATH/Orig/SignalPoints/Sig_450_400.json $JSON_PATH/Orig/SignalPoints/Sig_450_410.json $JSON_PATH/Orig/SignalPoints/Sig_450_420.json $JSON_PATH/Orig/SignalPoints/Sig_450_430.json $JSON_PATH/Orig/SignalPoints/Sig_450_440.json
mergeJSONs $JSON_PATH/stop475.json            $JSON_PATH/Orig/SignalPoints/Sig_475_395.json $JSON_PATH/Orig/SignalPoints/Sig_475_405.json $JSON_PATH/Orig/SignalPoints/Sig_475_415.json $JSON_PATH/Orig/SignalPoints/Sig_475_425.json $JSON_PATH/Orig/SignalPoints/Sig_475_435.json $JSON_PATH/Orig/SignalPoints/Sig_475_445.json $JSON_PATH/Orig/SignalPoints/Sig_475_455.json $JSON_PATH/Orig/SignalPoints/Sig_475_465.json
mergeJSONs $JSON_PATH/stop500.json            $JSON_PATH/Orig/SignalPoints/Sig_500_420.json $JSON_PATH/Orig/SignalPoints/Sig_500_430.json $JSON_PATH/Orig/SignalPoints/Sig_500_440.json $JSON_PATH/Orig/SignalPoints/Sig_500_450.json $JSON_PATH/Orig/SignalPoints/Sig_500_460.json $JSON_PATH/Orig/SignalPoints/Sig_500_470.json $JSON_PATH/Orig/SignalPoints/Sig_500_480.json $JSON_PATH/Orig/SignalPoints/Sig_500_490.json
mergeJSONs $JSON_PATH/stop525.json            $JSON_PATH/Orig/SignalPoints/Sig_525_445.json $JSON_PATH/Orig/SignalPoints/Sig_525_455.json $JSON_PATH/Orig/SignalPoints/Sig_525_465.json $JSON_PATH/Orig/SignalPoints/Sig_525_475.json $JSON_PATH/Orig/SignalPoints/Sig_525_485.json $JSON_PATH/Orig/SignalPoints/Sig_525_495.json $JSON_PATH/Orig/SignalPoints/Sig_525_505.json $JSON_PATH/Orig/SignalPoints/Sig_525_515.json
mergeJSONs $JSON_PATH/stop550.json            $JSON_PATH/Orig/SignalPoints/Sig_550_470.json $JSON_PATH/Orig/SignalPoints/Sig_550_480.json $JSON_PATH/Orig/SignalPoints/Sig_550_490.json $JSON_PATH/Orig/SignalPoints/Sig_550_500.json $JSON_PATH/Orig/SignalPoints/Sig_550_510.json $JSON_PATH/Orig/SignalPoints/Sig_550_520.json $JSON_PATH/Orig/SignalPoints/Sig_550_530.json $JSON_PATH/Orig/SignalPoints/Sig_550_540.json
mergeJSONs $JSON_PATH/stop575.json            $JSON_PATH/Orig/SignalPoints/Sig_575_495.json $JSON_PATH/Orig/SignalPoints/Sig_575_505.json $JSON_PATH/Orig/SignalPoints/Sig_575_515.json $JSON_PATH/Orig/SignalPoints/Sig_575_525.json $JSON_PATH/Orig/SignalPoints/Sig_575_535.json $JSON_PATH/Orig/SignalPoints/Sig_575_545.json $JSON_PATH/Orig/SignalPoints/Sig_575_555.json $JSON_PATH/Orig/SignalPoints/Sig_575_565.json
mergeJSONs $JSON_PATH/stop600.json            $JSON_PATH/Orig/SignalPoints/Sig_600_520.json $JSON_PATH/Orig/SignalPoints/Sig_600_530.json $JSON_PATH/Orig/SignalPoints/Sig_600_540.json $JSON_PATH/Orig/SignalPoints/Sig_600_550.json $JSON_PATH/Orig/SignalPoints/Sig_600_560.json $JSON_PATH/Orig/SignalPoints/Sig_600_570.json $JSON_PATH/Orig/SignalPoints/Sig_600_580.json $JSON_PATH/Orig/SignalPoints/Sig_600_590.json
mergeJSONs $JSON_PATH/stop625.json            $JSON_PATH/Orig/SignalPoints/Sig_625_545.json $JSON_PATH/Orig/SignalPoints/Sig_625_555.json $JSON_PATH/Orig/SignalPoints/Sig_625_565.json $JSON_PATH/Orig/SignalPoints/Sig_625_575.json $JSON_PATH/Orig/SignalPoints/Sig_625_585.json $JSON_PATH/Orig/SignalPoints/Sig_625_595.json $JSON_PATH/Orig/SignalPoints/Sig_625_605.json $JSON_PATH/Orig/SignalPoints/Sig_625_615.json
mergeJSONs $JSON_PATH/stop650.json            $JSON_PATH/Orig/SignalPoints/Sig_650_570.json $JSON_PATH/Orig/SignalPoints/Sig_650_580.json $JSON_PATH/Orig/SignalPoints/Sig_650_590.json $JSON_PATH/Orig/SignalPoints/Sig_650_600.json $JSON_PATH/Orig/SignalPoints/Sig_650_610.json $JSON_PATH/Orig/SignalPoints/Sig_650_620.json $JSON_PATH/Orig/SignalPoints/Sig_650_630.json $JSON_PATH/Orig/SignalPoints/Sig_650_640.json
mergeJSONs $JSON_PATH/stop675.json            $JSON_PATH/Orig/SignalPoints/Sig_675_595.json $JSON_PATH/Orig/SignalPoints/Sig_675_605.json $JSON_PATH/Orig/SignalPoints/Sig_675_615.json $JSON_PATH/Orig/SignalPoints/Sig_675_625.json $JSON_PATH/Orig/SignalPoints/Sig_675_635.json $JSON_PATH/Orig/SignalPoints/Sig_675_645.json $JSON_PATH/Orig/SignalPoints/Sig_675_655.json $JSON_PATH/Orig/SignalPoints/Sig_675_665.json
mergeJSONs $JSON_PATH/stop700.json            $JSON_PATH/Orig/SignalPoints/Sig_700_620.json $JSON_PATH/Orig/SignalPoints/Sig_700_630.json $JSON_PATH/Orig/SignalPoints/Sig_700_640.json $JSON_PATH/Orig/SignalPoints/Sig_700_650.json $JSON_PATH/Orig/SignalPoints/Sig_700_660.json $JSON_PATH/Orig/SignalPoints/Sig_700_670.json $JSON_PATH/Orig/SignalPoints/Sig_700_680.json $JSON_PATH/Orig/SignalPoints/Sig_700_690.json
mergeJSONs $JSON_PATH/stop725.json            $JSON_PATH/Orig/SignalPoints/Sig_725_645.json $JSON_PATH/Orig/SignalPoints/Sig_725_655.json $JSON_PATH/Orig/SignalPoints/Sig_725_665.json $JSON_PATH/Orig/SignalPoints/Sig_725_675.json $JSON_PATH/Orig/SignalPoints/Sig_725_685.json $JSON_PATH/Orig/SignalPoints/Sig_725_695.json $JSON_PATH/Orig/SignalPoints/Sig_725_705.json $JSON_PATH/Orig/SignalPoints/Sig_725_715.json
mergeJSONs $JSON_PATH/stop750.json            $JSON_PATH/Orig/SignalPoints/Sig_750_670.json $JSON_PATH/Orig/SignalPoints/Sig_750_680.json $JSON_PATH/Orig/SignalPoints/Sig_750_690.json $JSON_PATH/Orig/SignalPoints/Sig_750_700.json $JSON_PATH/Orig/SignalPoints/Sig_750_710.json $JSON_PATH/Orig/SignalPoints/Sig_750_720.json $JSON_PATH/Orig/SignalPoints/Sig_750_730.json $JSON_PATH/Orig/SignalPoints/Sig_750_740.json
mergeJSONs $JSON_PATH/stop775.json            $JSON_PATH/Orig/SignalPoints/Sig_775_695.json $JSON_PATH/Orig/SignalPoints/Sig_775_705.json $JSON_PATH/Orig/SignalPoints/Sig_775_715.json $JSON_PATH/Orig/SignalPoints/Sig_775_725.json $JSON_PATH/Orig/SignalPoints/Sig_775_735.json $JSON_PATH/Orig/SignalPoints/Sig_775_745.json $JSON_PATH/Orig/SignalPoints/Sig_775_755.json $JSON_PATH/Orig/SignalPoints/Sig_775_765.json
mergeJSONs $JSON_PATH/stop800.json            $JSON_PATH/Orig/SignalPoints/Sig_800_720.json $JSON_PATH/Orig/SignalPoints/Sig_800_730.json $JSON_PATH/Orig/SignalPoints/Sig_800_740.json $JSON_PATH/Orig/SignalPoints/Sig_800_750.json $JSON_PATH/Orig/SignalPoints/Sig_800_760.json $JSON_PATH/Orig/SignalPoints/Sig_800_770.json $JSON_PATH/Orig/SignalPoints/Sig_800_780.json $JSON_PATH/Orig/SignalPoints/Sig_800_790.json

# Make a json with all samples
mergeJSONs $JSON_PATH/allSamples.json         $JSON_PATH/allMC.json $JSON_PATH/Orig/Data.json $JSON_PATH/allPD.json $JSON_PATH/Orig/DeltaM.json $JSON_PATH/stop250.json $JSON_PATH/stop275.json $JSON_PATH/stop300.json $JSON_PATH/stop325.json $JSON_PATH/stop350.json $JSON_PATH/stop375.json $JSON_PATH/stop400.json $JSON_PATH/stop425.json $JSON_PATH/stop450.json $JSON_PATH/stop475.json $JSON_PATH/stop500.json $JSON_PATH/stop525.json $JSON_PATH/stop550.json $JSON_PATH/stop575.json $JSON_PATH/stop600.json $JSON_PATH/stop625.json $JSON_PATH/stop650.json $JSON_PATH/stop675.json $JSON_PATH/stop700.json $JSON_PATH/stop725.json $JSON_PATH/stop750.json $JSON_PATH/stop775.json $JSON_PATH/stop800.json
mergeJSONs $JSON_PATH/allSamplesSwap.json     $JSON_PATH/allMC.json $JSON_PATH/Orig/DataSingleLepton.json  $JSON_PATH/Orig/DeltaM.json $JSON_PATH/stop250.json $JSON_PATH/stop275.json $JSON_PATH/stop300.json $JSON_PATH/stop325.json $JSON_PATH/stop350.json $JSON_PATH/stop375.json $JSON_PATH/stop400.json $JSON_PATH/stop425.json $JSON_PATH/stop450.json $JSON_PATH/stop475.json $JSON_PATH/stop500.json $JSON_PATH/stop525.json $JSON_PATH/stop550.json $JSON_PATH/stop575.json $JSON_PATH/stop600.json $JSON_PATH/stop625.json $JSON_PATH/stop650.json $JSON_PATH/stop675.json $JSON_PATH/stop700.json $JSON_PATH/stop725.json $JSON_PATH/stop750.json $JSON_PATH/stop775.json $JSON_PATH/stop800.json

mergeJSONs $JSON_PATH/allSamplesQuick.json    $JSON_PATH/allMCQuick.json $JSON_PATH/Orig/Data.json
#$JSON_PATH/stop250.json $JSON_PATH/stop275.json $JSON_PATH/stop300.json $JSON_PATH/stop325.json $JSON_PATH/stop350.json $JSON_PATH/stop375.json $JSON_PATH/stop400.json $JSON_PATH/stop425.json $JSON_PATH/stop450.json $JSON_PATH/stop475.json $JSON_PATH/stop500.json $JSON_PATH/stop525.json $JSON_PATH/stop550.json $JSON_PATH/stop575.json $JSON_PATH/stop600.json $JSON_PATH/stop625.json $JSON_PATH/stop650.json $JSON_PATH/stop675.json $JSON_PATH/stop700.json $JSON_PATH/stop725.json $JSON_PATH/stop750.json $JSON_PATH/stop775.json $JSON_PATH/stop800.json
# $JSON_PATH/Orig/DeltaM.json

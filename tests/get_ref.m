function get_ref(model_name)
% simulate the model_name and generate the reference result

load_system(model_name);

stop_time = str2double(get_param(model_name, 'StopTime'));

simOut = sim(model_name, 'StopTime', num2str(stop_time * 1.01));

close_system(model_name, 0);

yout = simOut.get('yout');

s = size(yout.signals);
s(2) = s(2) + 1;

ref = zeros(size(yout.time, 1), 1 + size(yout.signals, 2));

ref(:,1) = yout.time;
header = 'time';

for i = 1:size(yout.signals, 2)
    header = [header ',' yout.signals(i).blockName(numel(model_name)+2:end)];
    ref(:,i+1) = yout.signals(i).values;
end

file_name = [model_name '_ref.csv'];

fid = fopen(file_name,'w'); 
fprintf(fid, '%s\n', header);
fclose(fid);

dlmwrite(file_name, ref, '-append');

end
